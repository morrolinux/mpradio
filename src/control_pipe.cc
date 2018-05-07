#include <iostream>
#include <string>
#include <fcntl.h>
using namespace std;
#include "control_pipe.h"

#define CTL_BUFFER_SIZE 100

FILE *f_ctl;
string command;

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
    if(res == NULL) return -1;

    command = res;
    cout<<command<<endl;
    
    return 0;
}

int close_control_pipe() {
    if(f_ctl) return fclose(f_ctl);
    else return 0;
}
