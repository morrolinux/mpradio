#!/bin/bash

INTERVAL=$(crudini --get /pirateradio/pirateradio.config RDS updateInterval)
JUMP=$(crudini --get /pirateradio/pirateradio.config RDS charsJump)

sleep $INTERVAL

getTitle(){
	cat /home/pi/now_playing
}

while true
do
	title=$(getTitle)
	echo $title
        title_lenght=$(echo $title|wc -c)
	finish=$((title_lenght+JUMP)) 

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
