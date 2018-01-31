#!/bin/bash

sleep 5
JUMP=6

while true
do
        title="$(cat /home/pi/now_playing)"
        title_lenght=$(echo $title|wc -c)

        echo "TA ON" >/home/pi/rds_ctl
        echo "TA $title" >/home/pi/rds_ctl

	finish=$((title_lenght+JUMP)) 

        for i in $(seq 9 $JUMP $finish);
        do
                legacytitle="$(echo "$title" |cut -c $(($i-8))-$i)"
                echo "PS $legacytitle" >/home/pi/rds_ctl
                sleep 5
        done

        echo "PS $title" >/home/pi/rds_ctl
	sleep 5
        
done

