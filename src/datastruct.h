#pragma once

/** setting datastructure */

struct settings{
	string freq;
	string storage;
	string format;
	string btGain;
	string storageGain;
	bool shuffle;
	bool persistentPlaylist;
	bool resumePlayback;
	int rdsUpdateInterval;
	int rdsCharsJump;
};
