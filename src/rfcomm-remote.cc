/*
 * YOU NEED TO REGISTER THE SERIAL PORT SERVICE BEFORE RUNNING THIS.
 * sudo sdptool add --channel=1 SP
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
using namespace std;

#define QUEUE 10

constexpr auto MPRADIO_CTL= "/home/pi/mpradio_ctl";
constexpr auto CHANNEL=1;

int fd;

int open_control_pipe(const char *filename) {
	fd = open(filename, O_WRONLY | O_NONBLOCK);
	if(fd == -1){
		cout<<"error opening control pipe. is mpradio running?"<<endl;
		return -1;
	}
	return 0;
}

int write_pipe(string message){
	open_control_pipe(MPRADIO_CTL);
	message = message+"\n";
	if(write(fd, message.c_str(), strlen(message.c_str())) < 0 ){
		cout<<"\n Error writing to control pipe: "<<strerror(errno)<<endl;
		//printf("\n Error wiriting to pipe: %s %d \n", strerror(errno),strerror(fd));
		return -1;
	}
	return 0;
}

void handle(string message){
	size_t found = message.find_first_of(" ");
	string command = message.substr(0,found);
	string arguments = message.substr(found+1);	//PS: arguments == command if no args are provided
	
	if (command.compare("system") == 0){
		system(arguments.c_str());
	}else{
		write_pipe(command);
	}
}

int main(void)
{
	system("sdptool add --channel=CHANNEL SP");
	int sock, client, bytes_read;
	unsigned int alen;
	struct sockaddr_rc addr;
	char buf[1024] = { 0 };
	

	if( (sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0){
	   	perror("socket");
	    	exit(1);
	}
	
	bdaddr_t tmp = { };
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, &tmp);
	addr.rc_channel = htobs(CHANNEL);
	alen = sizeof(addr);
	
	if(bind(sock, (struct sockaddr *)&addr, alen) < 0){
	    	perror("bind");
	    	exit(1);
	}
	
	while(1){
	
	   	 listen(sock,QUEUE);
	   	 printf("Waiting for connections...\n\n");  
	
	   	 if(client = accept(sock, (struct sockaddr *)&addr, &alen)){
	   	    	 printf("Got a connection! \n");
			 
	   	    	 // read data from the client
	   	    	 while(bytes_read = read(client, buf, sizeof(buf))){
	   	    		 if( bytes_read > 0 ){
	   	    		     	printf("received: %s\n", buf);
	   	    	         	handle(string(buf));
	   	    		 }else if( bytes_read < 0 ){
	   	    		     	printf("close connection\n");
	   	    		     	close(client);
	   	    		     	client=0;
	   	    	         	break;
	   	    		 }
	   	    		 //printf("bytes_read [%d]\n",bytes_read);
				 memset(buf,0,sizeof(buf));	//flushing the buffer for next command
	   	    	 }
	   	 }

	}

	close(sock);
	return 0;
}

