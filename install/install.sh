#!/bin/bash

CORE_INSTALL=""

# Get the root of the git repository
cd "${0%/*}"
GIT_ROOT="$(git rev-parse --show-toplevel)"

# Write this as an update script to ensure that bash
# knows how to complete updating before it starts to.
# Otherwise, it might begin corrupt execution using sudo ...
update_script() {
	sleep 1
	rm -rf /usr/local/src/PiFmAdv	# Make sure PiFm is being updated as well
	rm -rf /usr/local/src/PiFmRds
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

if [[ $1 == "core" ]] ; then
	CORE_INSTALL="true"
elif [[ $1 == "update" ]] ; then
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
	# Note: this does not handle directories.
	# This is good, since `sudo rm -rf` is almost never a good thing to run, ever
	handle() {
		echo "Removing $2 ..."
		rm -f $2 ;
	}
	
	systemctl stop mpradio
else
	INSTALL="install"
	handle() {
		echo "Copying $1 to $2 ..."
		cp -f $@ ;
	}
fi

if [[ $remove != "some" && $CORE_INSTALL != "true" ]]; then
	#Installing software dependencies...
	apt-get -y $INSTALL git libsndfile1-dev libbluetooth-dev bluez pi-bluetooth python-gobject python-gobject-2 bluez-tools sox crudini libsox-fmt-mp3 python-dbus bluealsa obexpushd libid3-dev unp
	apt-get -y remove pulseaudio
fi

INPUT="/etc/udev/rules.d/99-input.rules"
inputline=$(grep "bluetooth" "$INPUT" -n|cut -d: -f1)
if [[ $inputline == "" ]]; then
	echo "KERNEL==\"input[0-9]*\", RUN+=\"/usr/lib/udev/bluetooth\"" >> "$INPUT"
elif [[ $remove ]]; then
	sed -i.bak -e "${inputline}d" "$INPUT"
fi

#Installing needed files and configurations
handle "$GIT_ROOT"/install/mpradio-pushbutton-skip.py	 /bin/mpradio-pushbutton-skip.py
handle "$GIT_ROOT"/install/need2recompile.sh			 /bin/need2recompile.sh
handle "$GIT_ROOT"/install/bt-setup.sh					 /bin/bt-setup.sh
handle "$GIT_ROOT"/install/mpradio-legacyRDS.sh			 /bin/mpradio-legacyRDS.sh
handle "$GIT_ROOT"/install/simple-agent					 /bin/simple-agent

### deprecated since Read-Only mode and blueooth companion app
# handle "$GIT_ROOT"/install/100-usb.rules				 /etc/udev/rules.d/100-usb.rules

mkdir -p												 /pirateradio
mkdir -p												 /usr/lib/udev
handle "$GIT_ROOT"/install/bluetooth					 /usr/lib/udev/bluetooth
handle "$GIT_ROOT"/install/audio.conf					 /etc/bluetooth/audio.conf
handle "$GIT_ROOT"/install/main.conf					 /etc/bluetooth/main.conf
handle "$GIT_ROOT"/install/mpradio-bt@.service			 /lib/systemd/system/mpradio-bt@.service

#do not update the config file if a core update is performed
if [[ $CORE_INSTALL != "true" ]] ; then
	handle "$GIT_ROOT"/install/pirateradio.config /pirateradio/pirateradio.config --backup --suffix=.bak
fi

#compile and $INSTALL mpradio_cc
if [[ $remove ]]; then
	echo "not compiling before uninstall"
else
	cd ../src/

	mkdir libs
	git clone https://github.com/benhoyt/inih.git libs/inih

	make clean
	make -j $(nproc) 

 	g++ -o rfcomm-remote rfcomm-remote.cc -lbluetooth
fi

systemctl stop mpradio
handle "$GIT_ROOT"/src/mpradio /bin/mpradio
handle "$GIT_ROOT"/src/rfcomm-remote /usr/local/bin/rfcomm-remote

if [[ $remove ]]; then
	systemctl disable mpradio.service
	systemctl disable bluealsa.service
	systemctl disable simple-agent.service
	systemctl disable bt-setup.service
	systemctl disable need2recompile.service
	systemctl disable mpradio-legacy-rds.service
	systemctl disable mpradio-pushbutton-skip.service
	systemctl disable obexpushd.service
	systemctl disable rfcomm
fi

# Installing service units, or uninstalling them.
handle "$GIT_ROOT"/install/need2recompile.service			 /etc/systemd/system/need2recompile.service
handle "$GIT_ROOT"/install/mpradio-legacy-rds.service		 /etc/systemd/system/mpradio-legacy-rds.service
handle "$GIT_ROOT"/install/bt-setup.service					 /etc/systemd/system/bt-setup.service
handle "$GIT_ROOT"/install/mpradio.service					 /etc/systemd/system/mpradio.service
handle "$GIT_ROOT"/install/simple-agent.service				 /etc/systemd/system/simple-agent.service
handle "$GIT_ROOT"/install/mpradio-pushbutton-skip.service	 /etc/systemd/system/mpradio-pushbutton-skip.service
handle "$GIT_ROOT"/install/obexpushd.service				 /etc/systemd/system/obexpushd.service
handle "$GIT_ROOT"/install/dbus-org.bluez.service			 /etc/systemd/system/dbus-org.bluez.service
handle "$GIT_ROOT"/install/file_storage.sh					 /bin/file_storage.sh
handle "$GIT_ROOT"/install/rfcomm.service					 /etc/systemd/system/rfcomm.service

# Override bluealsa.service's configuration to specify exactly how bluealsa should run
# ... or repeal the override, if we are removing
mkdir -p /etc/systemd/system/bluealsa.service.d/
handle "$GIT_ROOT"/install/override.conf /etc/systemd/system/bluealsa.service.d/override.conf

if [[ ! $remove ]]; then
	systemctl enable mpradio.service
	systemctl enable bluealsa.service
	systemctl enable simple-agent.service
	systemctl enable bt-setup.service
	systemctl enable need2recompile.service
	systemctl enable mpradio-legacy-rds.service
	systemctl enable mpradio-pushbutton-skip.service
	systemctl enable obexpushd.service
	systemctl enable rfcomm

fi

#Installing PiFmRDS...

if [[ $remove || $CORE_INSTALL == "true" ]]; then
	echo "not compiling before uninstall"
else
	cd /usr/local/src/
	git clone https://github.com/ChristopheJacquet/PiFmRds.git 
	cd PiFmRds/src
	make clean
	make -j $(nproc)

	cd /usr/local/src/
	git clone https://github.com/Miegl/PiFmAdv.git
	cd PiFmAdv/src
	make clean
	make -j $(nproc)
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

systemctl daemon-reload		#needed when changing systemd unit files

echo "Completed! Rebooting in 5 seconds..."
sleep 5 && reboot
