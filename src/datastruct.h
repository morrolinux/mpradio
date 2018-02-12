#pragma once

/** setting datastructure */

struct settings{
	string freq;
	string storage;
	string format;
	string btGain;
	string storageGain;
	string output;
	bool shuffle;
	bool persistentPlaylist;
	bool resumePlayback;
	int rdsUpdateInterval;
	int rdsCharsJump;
};

struct playbackStatus{
	string songName;
	int songIndex=0;
	int playbackPosition=0;
	bool resumed=false;
};
