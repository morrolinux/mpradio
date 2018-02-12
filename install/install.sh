#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

if [[ $1 == "remove" ]] ; then 
	remove="all"
else 
	remove=""
fi

if [[ $remove ]] ; then 
	INSTALL="remove"
	CP="rm -f"
	systemctl stop mpradio
else
	INSTALL="install"
	CP="cp -f"
fi

#Installing software dependencies...
apt-get -y $INSTALL bluez pi-bluetooth python-gobject python-gobject-2 bluez-tools sox crudini libsox-fmt-mp3 python-dbus bluealsa
apt-get -y remove pulseaudio

#Installing software needed to compile PiFmRDS..
apt-get -y $INSTALL git libsndfile1-dev

#Setting rules...
BLACKLIST="/etc/modprobe.d/blacklist.conf"
blacklistline=$(grep "blacklist snd_bcm2835" $BLACKLIST -n|cut -d: -f1)
if [[ $blacklistline == "" ]]; then
	echo "blacklist snd_bcm2835" >> $BLACKLIST
	echo "blacklist ipv6" >> $BLACKLIST
else
	if [[ $remove ]]; then
		sed -i.bak -e "${blacklistline}d" $BLACKLIST
	fi
fi

INPUT="/etc/udev/rules.d/99-input.rules"
inputline=$(grep "bluetooth" $INPUT -n|cut -d: -f1)
if [[ $inputline == "" ]]; then
	echo "KERNEL==\"input[0-9]*\", RUN+=\"/usr/lib/udev/bluetooth\"" >> $INPUT
else
	if [[ $remove ]]; then
		sed -i.bak -e "${inputline}d" $INPUT
	fi
fi

#Installing needed files and configurations
${CP} mpradio-pushbutton-skip.py /bin/mpradio-pushbutton-skip.py
${CP} need2recompile.sh /bin/need2recompile.sh
${CP} mpshutdown.sh /sbin/mpshutdown.sh && chmod +x /sbin/mpshutdown.sh
${CP} bt-setup.sh /bin/bt-setup.sh
${CP} mpradio-legacyRDS.sh /bin/mpradio-legacyRDS.sh
${CP} simple-agent /bin/simple-agent
${CP} 100-usb.rules /etc/udev/rules.d/100-usb.rules
mkdir /pirateradio
${CP} -f ../install/pirateradio.config /pirateradio/pirateradio.config

mkdir -p /usr/lib/udev
${CP} bluetooth /usr/lib/udev/bluetooth
${CP} audio.conf /etc/bluetooth/audio.conf
${CP} main.conf /etc/bluetooth/main.conf
${CP} asoundrc /home/pi/.asoundrc
${CP} mpradio-bt@.service /lib/systemd/system/mpradio-bt@.service

#compile and $INSTALL mpradio_cc
if [[ $remove ]]; then
	echo "not compiling before uninstall"
else
	cd ../src/

	mkdir libs
	git clone https://github.com/benhoyt/inih.git libs/inih

	make clean
	make
fi

${CP} mpradio /home/pi/mpradio

#Installing service units...
cp -f ../install/need2recompile.service /etc/systemd/system/need2recompile.service
cp -f ../install/mpradio-legacy-rds.service /etc/systemd/system/mpradio-legacy-rds.service
cp -f ../install/bt-setup.service /etc/systemd/system/bt-setup.service
cp -f ../install/mpradio.service /etc/systemd/system/mpradio.service
cp -f ../install/simple-agent.service /etc/systemd/system/simple-agent.service
cp -f ../install/mpradio-pushbutton-skip.service /etc/systemd/system/mpradio-pushbutton-skip.service
if [[ $remove ]]; then
	systemctl disable mpradio.service
	systemctl disable bluealsa.service
	systemctl disable simple-agent.service
	systemctl disable bt-setup.service
	systemctl disable need2recompile.service
	systemctl disable mpradio-legacy-rds.service
	systemctl disable mpradio-pushbutton-skip.service
else
	systemctl enable mpradio.service
	systemctl enable bluealsa.service
	systemctl enable simple-agent.service
	systemctl enable bt-setup.service
	systemctl enable need2recompile.service
	systemctl enable mpradio-legacy-rds.service
	systemctl enable mpradio-pushbutton-skip.service
fi

#Installing PiFmRDS...

if [[ $remove ]]; then
	echo "not compiling before uninstall"
else
	cd /home/pi/
	git clone https://github.com/ChristopheJacquet/PiFmRds.git
	cd PiFmRds/src
	make clean
	make
fi

#Final configuration and perms...
FSTAB="/etc/fstab"
fstabline=$(grep "pirateradio" $FSTAB -n|cut -d: -f1)
if [[ $fstabline == "" ]]; then
	echo "/dev/sda1    /pirateradio    vfat    defaults,ro,nofail 0   0" >> $FSTAB
else
	if [[ $remove ]]; then
		sed -i.bak -e "${fstabline}d" $FSTAB
	fi
fi

usermod -a -G lp pi

echo "Installation completed! Rebooting in 10 seconds..."
sleep 10 && reboot
