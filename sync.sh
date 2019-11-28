#!/bin/bash

#send all output to syslog
 exec 1> >(exec logger -s -t $(basename $0)) 2>&1

#check if the remote share is mounted
MOUNTED=`mount -t cifs | wc -c`

if [ $MOUNTED -gt 0 ]
then
	echo "remote share mounted. Syncing..."
	rsync -av --delete /media/sambatest /home/luca/
	STATUS=$?
	if [ $STATUS -eq 0 ]
	then
		echo "sync successful"
		exit 0
	else
		echo "sync failed"
		exit 1
	fi
elif [ $MOUNTED -eq 0 ]
then
	echo "remote share not mounted. Check connection to server"
    exit 1
fi


