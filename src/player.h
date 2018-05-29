#pragma once

/** 
 * player routine for multimedia files on storage.
 * calls ::getsettings to fetch the needed settings and starts to play 
 */
int play_storage();

/** 
 * Bluetooth audio player. the only argument needed is the bluetooth device to listen to.
 */
int play_bt(string device);

void set_next_element();