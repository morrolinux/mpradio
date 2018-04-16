#!/bin/bash

INTERVAL=$(crudini --get /pirateradio/pirateradio.config RDS updateInterval)
JUMP=$(crudini --get /pirateradio/pirateradio.config RDS charsJump)
PATTERN=$(crudini --get /pirateradio/pirateradio.config RDS rdsPattern)

sleep $INTERVAL

getTitle(){

	# Verify now_playing exists before sourcing it
	# By sourcing it, we catch variables set by update_now_playing().
	[ -f /home/pi/now_playing ] && source /home/pi/now_playing

	# Now, pass PATTERN to eval() to compose the pattern together
	# E.g.: '%s: %s!!!' "$ARTIST_NAME" "$SONG_NAME" becomes "Stevie Wonder: Superstition!!!"
	eval printf $PATTERN
}

while true
do
	title=$(getTitle)
	echo $title
        title_length=$(echo $title|wc -c)
	finish=$((title_length+JUMP)) 

        for i in $(seq 9 $JUMP $finish);
        do
                legacytitle="$(echo "$title" |cut -c $(($i-8))-$i)"
                echo "PS $legacytitle" >/home/pi/rds_ctl
                sleep $INTERVAL
		if [[ $title != $(getTitle) ]]
		then
			break
		fi
        done
        
done
