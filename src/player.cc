#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <list>
#include <time.h>
#include <sys/stat.h>
using namespace std;
#include "datastruct.h"

extern settings s;
list<string> pqueue;
list<string>::iterator it;

void legacy_rds_init()
{
	remove("/home/pi/rds_ctl");
	mkfifo("/home/pi/rds_ctl",S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	chmod("/home/pi/rds_ctl",0777);
}

/**
 * creates a file list each time it is launched 
 * and saves the elements in pqueue (play queue)
 */

void get_list()
{
	FILE *fp;
	const int line_size=200;
	char line[line_size];
	string result;

        string cmd = "find " + s.storage + " -not -path \'*/\\.*\' -iname *." + s.format;
	fp = popen(cmd.c_str(), "r");

	while (fgets(line, line_size, fp))
		pqueue.push_back(line);

	pclose(fp);
}

int play_storage()
{
	bool repeat = true;
	srand (time(NULL));
	legacy_rds_init();
	ofstream playing;

	while(repeat){
		get_list();		/**< generate a file list */
		int next;
		int qsize=pqueue.size();
	
		string sox="sox -t mp3 -v 1.3 -r 48000 -G";
		string sox_params="-t wav -";
		string pifm1="/home/pi/PiFmRds/src/pi_fm_rds -ctl /home/pi/rds_ctl -ps";
		string pifm2="-rt";
		string pifm3="-audio - -freq";
		string songpath;
		
		if(qsize <= 0) repeat=false;
		
		while(qsize > 0)
		{
			next=rand() % qsize;		/**< extract a random file from the list */
			it=pqueue.begin();
			advance (it,next);
			songpath=*it;
			songpath.erase(songpath.size()-1);
			pqueue.erase(it);
			qsize--;
	
			size_t found = songpath.find_last_of("/");	/**< extract song name out of the absolute file path */
	  		string songname=songpath.substr(found+1);
	
			string cmdline=sox+" "+"\""+songpath+"\""+" "+sox_params+" | "+\
				pifm1+" "+"\""+songname+"\""+" "+pifm2+" "+"\""+songname+"\""+" "+pifm3+" "+s.freq;
	

			playing.open("/home/pi/now_playing");
			playing<<songname;
			playing.close();

			system(cmdline.c_str());
		}
	}
	return 0;
}

int play_bt(string device)
{
	
	string cmdline="arecord -D bluealsa -f cd -c 2 | sox -t raw -v 1.7 -G -b 16 -e signed -c 2 -r 44100 - -t wav - | sudo /home/pi/PiFmRds/src/pi_fm_rds -ps 'BLUETOOTH' -rt 'A2DP BLUETOOTH' -freq "+s.freq+" -audio -";

	system(cmdline.c_str());
	return 0;
}

