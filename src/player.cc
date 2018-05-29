#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;
#include "datastruct.h"
#include "files.h"
#include "control_pipe.h"

constexpr int microseconds = 500000;

extern settings s;
playbackStatus ps;
list<string>::iterator it;
list<string> pqueue;
int qsize;

void set_next_element()
{
	if(ps.reloading) return;
	qsize=pqueue.size();

	/** determine how the next song should be choosen */
	if(s.resumePlayback && !ps.resumed){
		//load_playback_status();    //already called at init stage
	}else if(s.shuffle){
		ps.songIndex=(rand() % qsize);
	}else{
		ps.songIndex=0;
	}

	/** set ps.songPath to the corresponding playlist index */
	it=pqueue.begin();
	advance (it,ps.songIndex);
	ps.songPath=*it;
}

/**
  * DO NOT HINIBIT THIS FUNCTION
  */
string trim_audio_track(const string &path)
{
	string trim="";
	int filesize=get_file_size(path);
	float duration=get_song_duration(path);
	int bs=get_file_bs(filesize,duration);
	if(bs == 0) bs=4;

	if(s.resumePlayback && !ps.resumed && ps.fileFormat != "flac"){
		if(ps.playbackPosition >= 5)		/**< resume few seconds back */
			ps.playbackPosition-=5;

		cout<<"seeking the track..."<<endl;
		trim="dd bs="+to_string(bs)+"k skip="+to_string(ps.playbackPosition)+" if=\""+path+"\" | ";
		ps.resumed=true;
	}else{
		trim="dd bs="+to_string(bs)+"k skip=0 if=\""+path+"\" | ";
		ps.playbackPosition=0;
	}

	return trim;
}

void set_output(string &output)
{
 	cout<<"setting audio output to "<<s.output<<endl;
	if(s.output == "FM" || s.output == "fm"){
		if(s.implementation == "pi_fm_adv")
			output=output+" --wait 0";
                return;
        }
	if(s.output == "ANALOG" || s.output == "analog"){
		output="aplay";
		return;
	}
	if(s.output == "PIPE" || s.output == "pipe"){
		output="cat > /home/pi/mpradio_stream";
	}
}

void set_effects(string &sox_params)
{
	if(s.btBoost)
		sox_params+="compand 0.3,1 6:-70,-60,-20 -5 -90 0.2 ";
	if(s.treble != "0")
		sox_params+="treble "+s.treble;
}

int fork_process(const string &cmd){
	pid_t pid = fork();
	if (pid == 0){
		//child process
		setsid();
		execlp("/bin/bash","bash","-c",cmd.c_str(),NULL);
		return 0;
	}else if(pid > 0){
		//parent process
		cout<<"parent process started\n";
		ps.pid=pid;
		int ps_status;
		usleep(microseconds);

		while (waitpid(pid, &ps_status, WNOHANG) == 0){
			usleep(microseconds);
		 	poll_control_pipe();
		}
	}else{
		//fork failed
		cout<<"fork failed \n";
		return 1;
	}
	cout<<"end of fork"<<endl;
	return 0;
}

int play_storage()
{
	bool repeat = true;
	srand (time(NULL));
	control_pipe_setup();

	load_playback_status();		/**< retrive songIndex and playbackPosition from ps file */
	thread persistPlayback (update_playback_status);	/**< this will keep updated the ps file with current index:position */
	open_control_pipe(MPRADIO_CTL);

	while(repeat){
		load_saved_list();
        qsize=pqueue.size();
	  	if(qsize <= 0){
            media_scan();										/**< generate a file list into pqueue */
			qsize=pqueue.size();
		}

		string sox="sox -v "+s.storageGain+" -r 44100 -G ";
		string sox_params=" - -t wav - ";
		set_effects(sox_params);
		string pifm1="/usr/local/bin/"+s.implementation+" "+s.opSwitch+"ctl /home/pi/rds_ctl "+s.opSwitch+"ps";
		string pifm2=s.opSwitch+"rt";
		string pifm3=s.opSwitch+"audio - "+s.opSwitch+"freq";
		string output="";

		if(qsize <= 0) repeat=false;		/**< infinite loop protection if no file are present */

		while(qsize > 0)
		{
			set_next_element();

			cout<<endl<<"PLAY: "<<ps.songPath<<endl;
			get_file_format(ps.songPath);
			read_tag_to_status(ps.songPath);

			string trim=trim_audio_track(ps.songPath);

			output=pifm1+" "+"\""+ps.songName+"\""+" "+pifm2+" "+"\""+ps.songName+"\""+" "+pifm3+" "+s.freq;
			set_output(output);			/**< change output device if specified */

			string cmdline=trim+sox+" -t "+ps.fileFormat+sox_params+" | "+output;
			cout<<"CMDLINE: "<<cmdline<<endl;

			update_now_playing();

			ps.reloading=false;
			fork_process(cmdline);

			if(ps.reloading) continue;
			cout<<"removing played song from playlist...\n";
			pqueue.erase(it);	/**< shorten the playlist and save it after playback */
			qsize--;
			save_list(qsize);
			remove(PSFILE);	/**< removing playback status file as not needed when playback ends */
		}
	}
	close_control_pipe();
	return 0;
}

int play_bt(string device)
{
	string sox_params="";
	string output="sudo /usr/local/bin/"+s.implementation+" "+s.opSwitch+"ps 'BLUETOOTH' "+s.opSwitch+"rt 'A2DP BLUETOOTH' "+s.opSwitch+"freq "+s.freq+" "+s.opSwitch+"audio -";
	set_output(output);			/**< change output device if specified */
	set_effects(sox_params);

	ps.songName = "Bluetooth";
	update_now_playing();

	string cmdline="arecord -D bluealsa:HCI=hci0,DEV="+device+" -f cd -c 2 | sox -t raw -v "+s.btGain+" -G -b 16 -e signed -c 2 -r 44100 - -t wav - "+sox_params+" | "+output;
	cout<<"CMDLINE: "<<cmdline<<endl;
	//string cmdline="arecord -D bluealsa -f cd -c 2 | sox -t raw -v "+s.btGain+" -G -b 16 -e signed -c 2 -r 44100 - -t wav - "+sox_params+" | "+output;  //legacy mode

	system(cmdline.c_str());
	return 0;
}
