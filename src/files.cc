#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <list>
#include <sys/stat.h>
#include <id3/tag.h>
#include <id3/misc_support.h>
using namespace std;
#include "datastruct.h"
#include "files.h"

#define SAFE_NULL(X) (NULL == X ? "" : X)
constexpr auto PLAYLIST = "/pirateradio/playlist";

extern settings s;
extern list<string> pqueue;
extern list<string>::iterator it;
extern playbackStatus ps;

int get_process_output_line(string cmd,string &output){

	int exitcode=0;
	FILE *fp;
	const int line_size=200;
	char line[line_size];
	string result;

	fp = popen(cmd.c_str(), "r");
	fgets(line, line_size, fp);
	output = line;
	int st = pclose(fp); 

	if(WIFEXITED(st)) 
		exitcode=WEXITSTATUS(st);

	if(exitcode != 0)
		output="";
	else
		output=output.erase(output.find('\n'));	//remove lewline char

	return exitcode;
}

/**
 * creates a file list each time it is launched 
 * and saves the elements in pqueue (play queue)
 */

void get_list()
{
	load_saved_list();		
	if(pqueue.size() != 0)
		return;

	FILE *fp;
	const int line_size=200;
	char line[line_size];
	string result;

        string cmd = "find " + s.storage + " -not -path \'*/\\.*\' -iname *." + s.format + "|sort -V";
	fp = popen(cmd.c_str(), "r");

	while (fgets(line, line_size, fp)){
		string s = line;
		s=s.erase(s.find('\n'));	//remove lewline char
		pqueue.push_back(s);
	}
	pclose(fp);
}


void save_list(int qsize)
{
	if(!s.persistentPlaylist)	/*< list is saved to file (or not) according to settings */
		return;

	if(qsize==0){
		remove(PLAYLIST);
		return;
	}

	ofstream list;
	list.open(PLAYLIST);

	it=pqueue.begin();

	for(int i=0; i<qsize; i++){
		list<<*it<<endl;
		advance (it,1);
	}
	list.close();
}

void load_saved_list()
{
	if(!s.persistentPlaylist)	/*< list is loaded from file (or not) according to settings */
		return;

	string line;
	ifstream list(PLAYLIST);
	if (list.is_open()){
		while ( getline (list,line) ){
			pqueue.push_back(line);
		}
		list.close();
	}
}

int get_file_size(string filename)
{
	struct stat stat_buf;
    	int rc = stat(filename.c_str(), &stat_buf);
    	return rc == 0 ? stat_buf.st_size : -1;
}

float get_song_duration(string path)
{
	string cmd="soxi -D \""+path+"\"";
	string s;
	if(get_process_output_line(cmd,s) !=0 )		
		return -1;		/**< make sure no valid duration is returned if the file does not exist/cannot be opened */

	float sd = strtof((s).c_str(),0);
	return sd;
}

int get_file_bs(int filesize,float fileduration)
{
	if(fileduration <= 0)
		return 0;		/**< 0 is an invalid bs for dd and will cause the pipe to fail, skipping to the next song */

	int bs = ((filesize/1000)/fileduration);
	cout<<"FILE SIZE: "<<filesize<<" DURATION: "<<fileduration<<" BS: "<<bs<<endl;
	return bs;
}

void load_playback_status()
{
	if(ps.resumed) return;		/**< don't do anything if already restored */

	ifstream psfile("/pirateradio/ps");
	if (psfile.is_open()){
		psfile>>ps.songIndex;
		psfile>>ps.playbackPosition;
		psfile.close();
	}else{
		ps.songIndex=0;
		ps.playbackPosition=0;
	}
}

/*! \brief Update the ~/now_playing file to communicate metadata of current song. 
 */
void update_now_playing()
{
	if(ps.repeat) return;
	ofstream playing;
	playing.open("/pirateradio/now_playing");
	playing<<"SONG_NAME='"<<ps.songName<<"'\n"
		   <<"SONG_YEAR='"<<ps.songYear<<"'\n"
		   <<"ALBUM_NAME='"<<ps.songAlbum<<"'\n"
		   <<"ARTIST_NAME='"<<ps.songArtist<<"'"
		   <<endl;
	playing.close();
}

void update_playback_status()
{
	if(!s.resumePlayback) return;
	if(ps.repeat);

	unsigned int seconds = 5;
	ofstream psfile;
	while (true){
		sleep(seconds);
		ps.playbackPosition+=5;
		psfile.open("/pirateradio/ps");
		psfile<<ps.songIndex<<" "<<ps.playbackPosition<<endl;
		psfile.close();
	}

}

/*! \brief Read the contents of the ID3tag on the file at songpath
 *         into playbackStatus ps, so that we can keep track of them.
 * @param[in]  songpath String representing the full filepath of current song.
 */
void read_tag_to_status(string songpath)
{
	if(ps.repeat) return;
	ID3_Tag tag(songpath.c_str());
	
	ps.songPath = songpath;
	ps.songName = SAFE_NULL(ID3_GetTitle( &tag ));
	ps.songArtist = SAFE_NULL(ID3_GetArtist( &tag ));
	ps.songAlbum = SAFE_NULL(ID3_GetAlbum( &tag ));
	ps.songYear = SAFE_NULL(ID3_GetYear( &tag ));

	if(ps.songName.empty()) {
		size_t found = songpath.find_last_of("/");	/**< extract song name out of the absolute file path */
		string songname=songpath.substr(found+1);
		ps.songName=songname;			
	}
}

void get_file_format(string songpath)
{
	//if(ps.repeat) return;
	size_t found = songpath.find_last_of(".");
	string format = songpath.substr(found+1);
	cout<<"FORMAT: "<<format<<endl;
	ps.fileFormat = format;
}


