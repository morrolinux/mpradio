#pragma once

constexpr auto RDS_CTL= "/home/pi/rds_ctl";
constexpr auto MPRADIO_CTL= "/home/pi/mpradio_ctl";
constexpr auto MPRADIO_STREAM = "/home/pi/mpradio_stream";
constexpr auto PSFILE = "/pirateradio/ps";
constexpr auto PLAYLIST = "/pirateradio/playlist";
constexpr auto NOW_PLAYING = "/pirateradio/now_playing";

/** setting datastructure */

struct settings{
	string implementation;
	string opSwitch;
	string freq;
	string storage;
	string format;
	string btGain;
	string treble;
	string storageGain;
	string output;
	string rdsStationName;
	bool shuffle;
	bool persistentPlaylist;
	bool resumePlayback;
	bool btBoost;
	int rdsUpdateInterval;
	int rdsCharsJump;
};

struct playbackStatus{
	string songPath;
	string songName;
	string songArtist;
	string songAlbum;
	string songYear;
	string fileFormat;
	pid_t pid;
	int songIndex=0;
	int playbackPosition=0;
	bool resumed=false;
	bool reloading=false;
};
