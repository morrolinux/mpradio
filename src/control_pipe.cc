#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;
#include "control_pipe.h"
#include "datastruct.h"
#include "files.h"
#define CTL_BUFFER_SIZE 100

FILE *f_ctl;
extern playbackStatus ps;
extern settings s;

/*
 * Opens a file (pipe) to be used to control the RDS coder, in non-blocking mode.
 */

int open_control_pipe(const char *filename) {
	int fd = open(filename, O_RDONLY | O_NONBLOCK);
    if(fd == -1) return -1;

	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	if( fcntl(fd, F_SETFL, flags) == -1 ) return -1;

	f_ctl = fdopen(fd, "r");
	if(f_ctl == NULL) return -1;

	return 0;
}


/*
 * Polls the control file (pipe), non-blockingly, and if a command is received,
 * processes it and updates the RDS data.
 */

int poll_control_pipe() {
    static char buf[CTL_BUFFER_SIZE];
    char *res = fgets(buf, CTL_BUFFER_SIZE, f_ctl);
    string::size_type sz;
    if(res == NULL) return -1;

    string input = string(res);
    input.erase(input.size() - 1);	//removing carriage return

    size_t found = input.find_first_of(" ");
    cout<<"found space at: "<<found<<endl;
    string command = input.substr(0,found);
    string arguments = input.substr(found+1);
    cout<<"command: "<<command<<" arguments: "<<arguments<<endl;

    if (command.compare("SKIP") == 0){
	    killpg(ps.pid,15);
    }else if(command.compare("SEEK") == 0){
	    if(!(arguments[0] == '+' || arguments[0] == '-')) return -1;

	    ps.repeat = true;

	    ps.playbackPosition = ps.playbackPosition + stoi(arguments,&sz);
	    if(ps.playbackPosition < 0)
		    ps.playbackPosition = 0;

	    s.resumePlayback = true;
	    ps.resumed = false;
	    killpg(ps.pid,15);
    }else if(command.compare("PLAY") == 0){		//TRIAL (works but meh..)
	    if(!(arguments[0] == '/')) return -1;
	    ps.songPath = arguments;
	    size_t found = arguments.find_last_of("/");
	    ps.songName = arguments.substr(found+1);
	    ps.repeat = true;
	    ps.playbackPosition = 0;
	    get_file_format(arguments);
    	    s.resumePlayback = true;
	    ps.resumed = false;
	    killpg(ps.pid,15);
}

    return 0;
}

int close_control_pipe() {
    if(f_ctl) return fclose(f_ctl);
    else return 0;
}
