#!/bin/bash

if [ "$#" -ne 1 ]; then
    	echo "Illegal number of parameters"
  	exit
fi

if [[ $1 == "ro" ]]
then
	sudo sed -i.bak '/^PARTUUID/ s/defaults/defaults,ro/' /etc/fstab
	sudo sed -i.bak 's/rootwait *$/rootwait noswap ro/' /boot/cmdline.txt
	echo "You need to reboot to make this effective."
elif [[ $1 == "rw" ]]
then
	sudo mount -o remount,rw /
	sudo sed -i.bak '/^PARTUUID/ s/defaults,ro/defaults/' /etc/fstab
	sudo sed -i.bak 's/rootwait noswap ro*$/rootwait/' /boot/cmdline.txt
	echo "RW effective now!"
fi

