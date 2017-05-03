#include <iostream>
using namespace std;

#include "fsrun.h"
#include "player.h"
#include "settings_provider.h"

/** /bin/su pi -c "parec -d $BT_DEVICE|sox -t raw -v 2.5 -G -b 16 -e signed -c 2 -r 44100 - -t wav - | sudo /home/pi/PiFmRds/src/pi_fm_rds -ps 'BLUETOOTH' -rt 'A2DP BLUETOOTH' -freq $FREQ -audio -"
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
