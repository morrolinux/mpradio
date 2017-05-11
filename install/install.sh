#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

#Installing software dependencies...
apt-get -y install bluez pulseaudio-module-bluetooth python-gobject python-gobject-2 bluez-tools sox crudini libsox-fmt-mp3 python-dbus

#Installing software needed to compile PiFmRDS..
apt-get -y install git libsndfile1-dev

#Setting rules...
echo "blacklist snd_bcm2835" >> /etc/modprobe.d/blacklist.conf
echo "blacklist ipv6" >> /etc/modprobe.d/blacklist.conf
echo "KERNEL==\"input[0-9]*\", RUN+=\"/usr/lib/udev/bluetooth\"" >> /etc/udev/rules.d/99-input.rules

#Installing needed files and configurations
cp bluezutils.py /bin/bluezutils.py
cp simple-agent /bin/simple-agent
echo "@reboot root /bin/simple-agent&" >> /etc/crontab
cp daemon.conf /etc/pulse/daemon.conf
mkdir /usr/lib/udev
cp bluetooth /usr/lib/udev/bluetooth
cp audio.conf /etc/bluetooth/audio.conf
cp main.conf /etc/bluetooth/main.conf

#compile and install mpradio_cc
cd ../src/
make clean
make
cp mpradio /home/pi/

#Installing service units...
cp ../install/mpradio.service /etc/systemd/system/mpradio.service
systemctl enable mpradio.service

#Installing PiFmRDS...
cd /home/pi/
git clone https://github.com/ChristopheJacquet/PiFmRds.git
cd PiFmRds/src
make clean
make

#Final configuration and perms...
echo "/dev/sda1    /pirateradio    vfat    defaults,ro,nofail 0   0" >> /etc/fstab
usermod -a -G lp pi

echo "Installation completed! Rebooting in 10 seconds..."
sleep 10 && reboot
