#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
using namespace std;

#include "datastruct.h"

#define FORMAT "mp3"
#define STORAGE "/pirateradio"
#define SETTINGS "/pirateradio/pirateradio.config"
#define DEFAULTFREQ "88.8"
#define DEFAULTSNAME "MPRADIO"
#define STRLEN 40

settings s;

/** fetch the key:value pair for the query section/key passed by argument */
string read(string section,string key)
{
	char * tmp= new char[STRLEN];
	FILE *fp;
	string s0="crudini --get";
	string s1=SETTINGS;
	string s2=section;
	string s3=key;
	string cmd=s0+" "+s1+" "+s2+" "+s3;
	fp = popen(cmd.c_str(), "r");
	fgets(tmp, STRLEN, fp);
	pclose(fp);
	string result=tmp;
	delete [] tmp;
	return result;
}

void getsettings()
{
	s.freq=read("pirateradio","frequency");
        if(s.freq== ""){
                cout<<"no frequency has been set. setting to default..."<<endl;
                s.freq=DEFAULTFREQ;
        }else{
                s.freq.erase(s.freq.size()-1);
        }
	cout<<s.freq<<endl;

        s.sname=read("pirateradio","stationname");
        if(s.sname== ""){
                cout<<"no stationname has been set. setting to default..."<<endl;
                s.sname=DEFAULTSNAME;
        }else{
                s.sname.erase(s.sname.size()-1);
        }
        cout<<s.sname<<endl;

	s.storage=STORAGE;
	cout<<s.storage<<endl;

	s.format="mp3";
}
