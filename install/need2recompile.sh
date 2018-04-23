#!/bin/bash

currentmodel="/sys/firmware/devicetree/base/model"
lastmodel="/etc/lastmodel"

touch $lastmodel
diff $currentmodel $lastmodel
equals=$?

if [[ $equals -eq 1 ]]
then
	echo different
	cp $currentmodel $lastmodel
	systemctl stop mpradio
	killall mpradio
	killall sox
	killall PiFmRDS

	mount -o remount rw /

	cd /usr/local/src/PiFmRds/src/
	make clean
	make

	cd /usr/local/src/PiFmAdv/src/
	make clean
	make

	cp /usr/local/src/PiFmRds/src/pi_fm_rds /usr/local/bin/pi_fm_rds
	cp /usr/local/src/PiFmAdv/src/pi_fm_adv /usr/local/bin/pi_fm_adv
	
	reboot
fi
