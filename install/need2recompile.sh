#!/bin/bash

currentmodel="/sys/firmware/devicetree/base/model"
lastmodel="/etc/lastmodel"

diff $currentmodel $lastmodel
equals=$?

if [[ $equals -eq 1 ]]
then
	echo different
	mount -o remount rw /

	cp $currentmodel $lastmodel
	systemctl stop mpradio
	killall mpradio
	killall sox
	killall pi_fm_rds
	killall pi_fm_adv
	
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
