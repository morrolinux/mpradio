#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
using namespace std;

#include "libs/inih/cpp/INIReader.h"
#include "datastruct.h"

#define FORMAT "mp3"
#define STORAGE "/pirateradio"
#define SETTINGS "/pirateradio/pirateradio.config"
#define DEFAULTFREQ "88.8"
#define STRLEN 40

settings s;

/** fetch the key:value pair for the query section/key passed by argument */
string read(string section,string key)
{
	INIReader reader("/pirateradio/pirateradio.config");
	if (reader.ParseError() < 0) {
		cout << "Can't load config file\n";
		return "";
	}

	return reader.Get(section,key,"ERR");
}

void getsettings()
{
	s.freq=read("PIRATERADIO","frequency");
        if(s.freq== ""){
                cout<<"no frequency has been set. setting to default..."<<endl;
                s.freq=DEFAULTFREQ;
        }
	cout<<s.freq<<endl;

	s.storage=STORAGE;
	cout<<s.storage<<endl;

	s.format="mp3";
}
