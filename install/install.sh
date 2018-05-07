#!/bin/bash

# Write this as an update script to ensure that bash
# knows how to complete updating before it starts to.
# Otherwise, it might begin corrupt execution using sudo ...
update_script() {
	sleep 1
	git fetch origin
	git reset --hard origin/master
	cd install # If we are already in the install directory, this is OK
	./install.sh
	exit $?
}

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

if [[ $1 == "update" ]] ; then
	update_script
elif [[ $1 == "remove" ]] ; then
	remove="all"
elif [[ $1 == "uninstall" ]] ; then
	remove="some"
else
	remove=""
fi

if [[ $remove ]] ; then
	INSTALL="remove"
	# N.b. this does not handle directories.
	# This is good, since `sudo rm -rf` is almost never a good thing to run, ever
	handle() { rm -f $2 ; }
	systemctl stop mpradio
else
	INSTALL="install"
	handle() { cp -f $@ ; }
fi

if [[ $remove != "some" ]]; then
	#Installing software dependencies...
	apt-get -y $INSTALL bluez pi-bluetooth python-gobject python-gobject-2 bluez-tools sox crudini libsox-fmt-mp3 python-dbus bluealsa obexpushd libid3-dev
	apt-get -y remove pulseaudio
	
	#Installing software needed to compile PiFmRDS..
	apt-get -y $INSTALL git libsndfile1-dev
fi

#Setting rules...
BLACKLIST="/etc/modprobe.d/blacklist.conf"
blacklistline="# Blacklist ipv6 for mpradio"
#No need to check first - we'll just remove blacklists starting with comment we added.
#$(grep "blacklist snd_bcm2835" $BLACKLIST -n|cut -d: -f1)
if [[ ! $remove ]]; then
	#echo "blacklist snd_bcm2835" >> $BLACKLIST	#no need to blacklist since pulseaudio removal
	echo "$blacklistline" >> "$BLACKLIST"
	echo "blacklist ipv6" >> "$BLACKLIST"
else
	# Remove all instances starting with comment line
	sed -i.bak "/${blacklistline}/,+1 d" "$BLACKLIST"
fi

INPUT="/etc/udev/rules.d/99-input.rules"
inputline=$(grep "bluetooth" "$INPUT" -n|cut -d: -f1)
if [[ $inputline == "" ]]; then
	echo "KERNEL==\"input[0-9]*\", RUN+=\"/usr/lib/udev/bluetooth\"" >> "$INPUT"
elif [[ $remove ]]; then
	sed -i.bak -e "${inputline}d" "$INPUT"
fi

#Installing needed files and configurations
handle mpradio-pushbutton-skip.py /bin/mpradio-pushbutton-skip.py
handle need2recompile.sh /bin/need2recompile.sh
handle mpshutdown.sh /sbin/mpshutdown.sh
handle bt-setup.sh /bin/bt-setup.sh
handle mpradio-legacyRDS.sh /bin/mpradio-legacyRDS.sh
handle simple-agent /bin/simple-agent
handle 100-usb.rules /etc/udev/rules.d/100-usb.rules
mkdir -p /pirateradio
handle ../install/pirateradio.config /pirateradio/pirateradio.config --backup --suffix=.bak

mkdir -p /usr/lib/udev
handle bluetooth /usr/lib/udev/bluetooth
handle audio.conf /etc/bluetooth/audio.conf
handle main.conf /etc/bluetooth/main.conf
handle asoundrc /home/pi/.asoundrc
handle mpradio-bt@.service /lib/systemd/system/mpradio-bt@.service

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

handle mpradio /bin/mpradio

if [[ $remove ]]; then
	systemctl disable mpradio.service
	systemctl disable bluealsa.service
	systemctl disable simple-agent.service
	systemctl disable bt-setup.service
	systemctl disable need2recompile.service
	systemctl disable mpradio-legacy-rds.service
	systemctl disable mpradio-pushbutton-skip.service
	systemctl disable obexpushd.service
fi

#Installing service units, or uninstalling them.
handle ../install/need2recompile.service /etc/systemd/system/need2recompile.service
handle ../install/mpradio-legacy-rds.service /etc/systemd/system/mpradio-legacy-rds.service
handle ../install/bt-setup.service /etc/systemd/system/bt-setup.service
handle ../install/mpradio.service /etc/systemd/system/mpradio.service
handle ../install/simple-agent.service /etc/systemd/system/simple-agent.service
handle ../install/mpradio-pushbutton-skip.service /etc/systemd/system/mpradio-pushbutton-skip.service
handle ../install/obexpushd.service /etc/systemd/system/obexpushd.service
handle ../install/dbus-org.bluez.service /etc/systemd/system/dbus-org.bluez.service
handle ../install/file_storage.sh /bin/file_storage.sh
handle ../install/rfcomm.service /etc/systemd/system/rfcomm.service

if [[ ! $remove ]]; then
	systemctl enable mpradio.service
	systemctl enable bluealsa.service
	systemctl enable simple-agent.service
	systemctl enable bt-setup.service
	systemctl enable need2recompile.service
	systemctl enable mpradio-legacy-rds.service
	systemctl enable mpradio-pushbutton-skip.service
	systemctl enable obexpushd.service
fi

#Installing PiFmRDS...

if [[ $remove ]]; then
	echo "not compiling before uninstall"
else
	cd /usr/local/src/
	git clone https://github.com/ChristopheJacquet/PiFmRds.git 
	cd PiFmRds/src
	make clean
	make

	cd /usr/local/src/
	git clone https://github.com/Miegl/PiFmAdv.git
	cd PiFmAdv/src
	make clean
	make

	
fi

handle /usr/local/src/PiFmRds/src/pi_fm_rds /usr/local/bin/pi_fm_rds
handle /usr/local/src/PiFmAdv/src/pi_fm_adv /usr/local/bin/pi_fm_adv

#Final configuration and perms...
FSTAB="/etc/fstab"
fstabline=$(grep "pirateradio" $FSTAB -n|cut -d: -f1)
if [[ $fstabline == "" ]]; then
	echo "/dev/sda1    /pirateradio    vfat    defaults,rw,nofail 0   0" >> $FSTAB
else
	if [[ $remove ]]; then
		sed -i.bak -e "${fstabline}d" $FSTAB
	fi
fi

usermod -a -G lp pi

# Edit /lib/systemd/system/bluetooth.service to enable BT services
# Credits to Patrick Hundal, hacks.mozilla.org
sed -i: 's|^Exec.*toothd$| \
ExecStart=/usr/lib/bluetooth/bluetoothd -C \
ExecStartPost=/usr/bin/sdptool add SP \
ExecStartPost=/bin/hciconfig hci0 piscan \
|g' /lib/systemd/system/bluetooth.service

echo PRETTY_HOSTNAME=mpradio > /etc/machine-info
cp -f /sys/firmware/devicetree/base/model /etc/lastmodel

if [[ $(grep "gpu_freq=250" /boot/config.txt) == "" ]]; then 
    echo "gpu_freq=250" >> /boot/config.txt
fi

echo "Completed! Rebooting in 5 seconds..."
sleep 5 && reboot
