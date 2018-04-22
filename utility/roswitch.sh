#!/bin/bash

if [ "$#" -ne 1 ]; then
        echo "Illegal number of parameters"
        exit
fi

if [[ $1 == "ro" ]]
then
        sudo sed -i.bak '/^PARTUUID/ s/defaults/defaults,ro/' /etc/fstab
        sudo sed -i.bak 's/rootwait *$/rootwait noswap ro/' /boot/cmdline.txt

        if [[ $(sudo grep "var" /etc/fstab) == "" ]]
        then
                sudo echo "tmpfs /var tmpfs noatime 0 0" >> /etc/fstab
        fi

        echo "You need to reboot to make this effective."
elif [[ $1 == "rw" ]]
then
        sudo mount -o remount,rw /
        sudo mount -o remount,rw /boot
        sudo sed -i.bak '/^PARTUUID/ s/defaults,ro/defaults/' /etc/fstab
        sudo sed -i.bak 's/rootwait noswap ro*$/rootwait/' /boot/cmdline.txt
        sudo umount -l /var/
	sudo sed -i '/tmpfs \/var tmpfs noatime 0 0/d' /etc/fstab
        echo "RW effective now!"
fi
