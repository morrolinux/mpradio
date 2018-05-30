# mpradio
Morrolinux's Pirate radio (PiFmRDS implementation with Bluetooth and mp3 support) for all Raspberry Pi models

Exclusively tested on Minimal Raspbian (ARM)

# Features
- [x] Safely shutdown your Pi by unplugging the USB stick
- [x] Persistent playlists (remember the playlist/playback status across reboots)
- [x] Resume track from its playback status hh:mm:ss
- [x] Shuffle on/off
- [x] Scrolling RDS to overcome 8-chars limitation 
- [x] Skip to the next song by pressing a push-button (GPIO-connected on pin 18)
- [x] Safely shutdown by holding the push-button (GPIO-connected on pin 18)
- [x] Stream audio over FM or 3.5mm Jack (Bluetooth speaker via jack audio output)
- [x] Send mp3 files to the Pi via Bluetooth
- [x] Bluetooth OTA file management on the Pi with applications such as "Bluetooth Explorer Lite"
- [x] Read metadata from the mp3 files 
- [x] Multiple file format support [mp3/wav/flac]
- [x] Read Only mode for saving sdcard from corruption when unplugging AC
- [x] PiFmAdv (optional)(experimental) implementation for better signal purity 
- [x] Control pipe commands (explained below)
- [ ] Display Android notifications over RDS?
- [ ] Bluetooth companion app for android (Work in progress...) 
- [ ] Automatically partition the sdcard for a dedicated mp3 storage space (instead of using a USB drive)

# Known issues
- Due to a design flaw in BCM43438 WIFI/BT chipset, you might need to disable WiFi if you experience BT audio stuttering on Pi Zero W and Pi 3: https://github.com/raspberrypi/linux/issues/1402
- Boot can take as long as 1m30s on the Pi 1 and 2 due to BT UART interface missing on the board.
  Reducing systemd timeout with `echo "DefaultTimeoutStartSec=40s" >> /etc/systemd/system.conf` should help
- PiFmAdv implementation (experimental) is really unstable on latest firmwares. installing an older one with
`rpi-update fa19f1c6b3ea5d09fb30e0b38a69199eed210bb4` should help

# Installation
First make sure your Raspbian is up to date:

` sudo apt-get update && sudo apt-get -y full-upgrade && sudo apt-get -y install git`

` git clone https://github.com/morrolinux/mpradio.git mpradio-master `

` cd mpradio-master/install && sudo ./install.sh`

# Configuration
By default, `mpradio` will always be running automatically after boot once installed. No additional configuration is needed.
However, you can change the FM streaming frequency (which is otherwise defaulted to 107.0) by placing a file named pirateradio.config in the root of a USB key (which of course, will need to stay plugged for the settings to be permanent)

pirateradio.config example:
```
[PIRATERADIO]
frequency=107.0
btGain=1.7            	;gain setting for bluetooth streaming
storageGain=1         	;gain setting for stored files streaming
output=fm		            ;[fm/analog] for FM output or 3.5 mm jack output
btBoost=false		        ;Enhance Bluetooth audio. This might add a little latency
implementation=pi_fm_rds	;[pi_fm_rds/pi_fm_adv] - pi_fm_adv (experimental) has a much cleaner sound but it's quite unstable

[PLAYLIST]
persistentPlaylist=true
resumePlayback=true   	;require persistentPlaylist to be enabled
shuffle=true 
fileFormat=all          ;which file formats to search for. [mp3/flac/wav/all]

[RDS]
updateInterval=3      				;seconds between RDS refresh. lower values could result in RDS being ignored by your radio receiver
charsJump=6           				;how many characters should shift between updates [1-8]
rdsPattern=$ARTIST_NAME - $SONG_NAME	;Pattern which is passed to eval() to produce title EG: $SONG_YEAR - $ALBUM_NAME

```
Optional: Protect your SD card from corruption by setting Read-Only mode.

use utility/roswitch.sh as follows:

`roswitch.sh ro` to enable read-ony (effective from next boot)

`roswitch.sh rw` to disable read-only (effective immediately)

# Usage
It (should) work out of the box. You need your mp3 files to be on a FAT32 USB stick (along with the `pirateradio.config` file if you want to override the default settings).
You can **safely** shut down the Pi by unplugging the stick and waiting for about 5 seconds until the status LED stops blinking.
If you enabled "persistentPlaylist" option, your Pi will never play the same song twice before consuming the full playlist.
If you add new songs on the USB stick, with "persistentPlaylist" enabled they won't be played until the current playlist is consumed. You can "rebuild" the playlist (looking for new recently added files) if needed:
- Boot your Pi with USB stick unplugged. (The current playlist will be erased.)
- Plug in your USB stick and unplug it again. (This will cause a shutdown.) 
- Power on your Pi once again, with the USB key in it.
- You're done! (`mpradio` will rebuild the playlist, counting the new files as well)
  
Also, please remember that (though it would be probably illegal) you can test FM broadcasting by plugging a 20cm wire on the **GPIO 4** of your Pi.

## Control pipe
You can perform certain operations while mpradio.service is running by simply writing to mpradio_ctl

Example:
* Play a song on demand: `echo "PLAY /absolute/path/to/song.mp3" > mpradio_ctl`
* Skip the current song:  `echo "SKIP" > mpradio_ctl`
* Seek the track forward or backwards: `echo "SEEK +10" > mpradio_ctl`  or  `echo "SEEK -10" > mpradio_ctl`
* Play all songs within a folder (via media scan): `echo "SCAN /absolute/path/to/folder/" > mpradio_ctl`

## Bluetooth companion app 

I'll post the source code once it's mature enough, but you can test an alpha version [here](http://www.mediafire.com/file/no44k0ovmfwfnga/mpradio_remote-0.1.apk)

NB: I haven't handled all corner conditions yet, so crashes may occour.

Screenshots:

<div class="row">
  <div class="column">
    <img src="/doc/app/UI.png" width="280" height="480">
  </div>
  <div class="column">
    <img src="/doc/app/swipe_song.png" width="280" height="480">
  </div>
  <div class="column">
    <img src="/doc/app/mpradio_settings.png" width="280" height="480">
  </div>
</div>


# Updating 
` cd mpradio-master/install && sudo ./install.sh update `

Or, if you prefer to be explicit:

`cd mpradio-master && git fetch origin && git reset --hard origin/master && cd install && sudo ./install.sh`

# Uninstallation / Removal
In order to remove `mpradio` along with the packages that come with it:

` cd mpradio-master/install && sudo ./install.sh remove `

This has the effect of removing dependency packages whether or not you still want them. If you would like to keep the packages that `mpradio` depends on, run the following instead:

` cd mpradio-master/install && sudo ./install.sh uninstall `

# Debugging / Troubleshooting
## Services
`mpradio` is launched as a service (via systemd) upon each boot.

To check whether the service is running or not: 

` $ sudo systemctl status mpradio `

To start or stop the service:

` $ sudo systemctl [start/stop] mpradio `

## Bluetooth

Bluetooth connection logs are found at ` /var/log/bluetooth_dev `.

If the Raspberry Pi is not showing up as a Bluetooth device, check whether the interface is UP, and that the `bt-setup` script is running:

` $ hciconfig `

` $ sudo systemctl status bt-setup `

If you are having issues with pairing Bluetooth for audio, please also check if `simple-agent` service is running:

` $ sudo systemctl status simple-agent `

If you are having issues with Bluetooth not connecting once it's paired, please check whether `bluealsa` is running or not:

` $ sudo systemctl status bluealsa `

A simple schematic of how things work together:

![MPRadio schematic](/doc/mpradio_schematic.png?raw=true "mpradio schematic")

# Warning and Disclaimer
`mpradio` relies on PiFmRds for FM-Streaming feature. Please note that in most states, transmitting radio waves without a state-issued licence specific to the transmission modalities (frequency, power, bandwidth, etc.) is illegal. Always use a shield between your radio receiver and the Raspberry. Never use an antenna. See PiFmRds Waring and Disclamer for more information.
