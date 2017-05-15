/**
 * \mainpage mpradio
 *
 * mpradio implements PiFmRDS, please check out their github and license.
 *
 * @author Moreno Razzoli
 */

/**
 * @file 
 * mpradio main
 */

#include <iostream>
using namespace std;

#include "fsrun.h"
#include "player.h"
#include "settings_provider.h"

/**
 * the main() checks whether an argument is passed or not
 * if an argument is passed, that is intended to be the 
 * bluetooth device to stream from. 
 * if no parameter is passed, mpradio will play mp3s 
 * from the folder specified in settings ( using ::getsettings )
*/
int main(int argc, char* argv[])
{
	need2recompile();
	getsettings();

	if(argc>1){
		cout<<argv[1]<<endl;
		play_bt(argv[1]);
	}else{
		cout<<"no bluetooth device provided, playing mp3s..."<<endl;
		play_storage();
	}

	return 0;
}
