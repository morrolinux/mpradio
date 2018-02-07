#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <list>
#include <sys/stat.h>
using namespace std;
#include "datastruct.h"
#include "files.h"

constexpr auto PLAYLIST = "/home/pi/playlist";

extern settings s;
extern list<string> pqueue;
extern list<string>::iterator it;
extern playbackStatus ps;

string get_process_output_line(string cmd){

	FILE *fp;
	const int line_size=200;
	char line[line_size];
	string result;

	fp = popen(cmd.c_str(), "r");

	fgets(line, line_size, fp);
	string s = line;
	s=s.erase(s.find('\n'));	//remove lewline char

	pclose(fp);
	return s;
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

        string cmd = "find " + s.storage + " -not -path \'*/\\.*\' -iname *." + s.format;
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
	string s = get_process_output_line(cmd);
	float sd = strtof((s).c_str(),0);
	return sd;
}

int get_file_bs(int filesize,float fileduration)
{
	int bs = ((filesize/1000)/fileduration);
	cout<<"FILE SIZE: "<<filesize<<" DURATION: "<<fileduration<<" BS: "<<bs<<endl;
	return bs;
}

void load_playback_status()
{
	if(ps.resumed) return;		/**< don't do anything if already restored */

	ifstream psfile("/home/pi/ps");
	if (psfile.is_open()){
		psfile>>ps.songIndex;
		psfile>>ps.playbackPosition;
		psfile.close();
	}else{
		ps.songIndex=0;
		ps.playbackPosition=0;
	}
}

void update_now_playing(string songname)
{
	ofstream playing;
	playing.open("/home/pi/now_playing");
	playing<<songname;
	playing.close();
}

void update_playback_status()
{
	if(!s.resumePlayback) return;

	unsigned int seconds = 5;
	ofstream psfile;
	while (true){
		sleep(seconds);
		ps.playbackPosition+=5;
		psfile.open("/home/pi/ps");
		psfile<<ps.songIndex<<" "<<ps.playbackPosition<<endl;
		psfile.close();
	}

}
