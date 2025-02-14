#!/bin/bash
 if [ -f "log_post" ]; then
   mv log_post log_backup
 fi

for i in $(seq 0 1 200)
do
#    FILE=dump-$i
#    if [ -f "$FILE" ]; then
#	mv dump-$i dump-$i.0
#	fi
	
#    FILE=dump-00$i
#    if [ -f "$FILE" ]; then
#	mv dump-00$i dump-$i.0
#	fi
	
    for j in $(seq 0 1 9)
	do
      for k in $(seq 0 1 9)
      do
         for m in $(seq 0 1 9)
         do
	        FILE=dump/dump-$i.$j$k$m
	        if [ -f "$FILE" ]; then
	        	cp dump/dump-$i.$j$k$m restartp
                        ./ep_post 2>> log_post
	      	fi
        done
      done
    done
done
 
