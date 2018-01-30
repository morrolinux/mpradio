#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
using namespace std;

#include "datastruct.h"

constexpr auto FORMAT = "mp3";
constexpr auto STORAGE = "/pirateradio";
constexpr auto SETTINGS = "/pirateradio/pirateradio.config";
constexpr auto DEFAULTFREQ = "88.8";
constexpr int STRLEN = 40;

settings s;

inline bool exists_test (const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

/** fetch the key:value pair for the query section/key passed by argument */
string read(string section,string key)
{
	if(! exists_test(SETTINGS) ) return "";
	
	char * tmp= new char[STRLEN];
    
	FILE *fp;

	string cmd=string("crudini --get ") + SETTINGS + " " + section + " " + key;
    
	fp = popen(cmd.c_str(), "r");
	fgets(tmp, STRLEN, fp);
	pclose(fp);
	string result=tmp;
	delete [] tmp;
	return result;
}

void getsettings()
{
	s.freq=read("PIRATERADIO","frequency");
    if(s.freq== ""){
            cout<<"no frequency has been set. setting to default..."<<endl;
            s.freq=DEFAULTFREQ;
    }else{
            s.freq.erase(s.freq.size()-1);
    }
	cout<<s.freq<<endl;

	s.storage=STORAGE;
	cout<<s.storage<<endl;

	s.format="mp3";
}
