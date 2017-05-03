#include <iostream>
#include <fstream>
#include <stdlib.h>
using namespace std;

#define FIRSTRUN "/firstrun"

bool recompile()
{
        system("systemctl stop mpradio.service");
        system("systemctl stop mpradio_rds.service");
        cout<<"first run: Recompiling PiFmRDS from source...";
        system("cd /home/pi/PiFmRds/src && make clean && make");
        system("systemctl start mpradio.service");
        system("systemctl start mpradio_rds.service");
        system("rm -f FIRSTRUN");
        return true;
}

bool need2recompile()
{
	ifstream first_run(FIRSTRUN);

	if (first_run)
	{
	        if(recompile())
			return true;
		else
			return false;
	}else{
	        return true;
	}
}
