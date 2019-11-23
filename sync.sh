#!/bin/bash

 exec 1> >(exec logger -s -t $(basename $0)) 2>&1

MOUNTED=`mount -t cifs | wc -c`

if [ $MOUNTED -gt 0 ]
then
	echo "remote share mounted. Syncing..."
	rsync -av --delete /media/sambatest /home/luca/
elif [ $MOUNTED -eq 0 ]
then
	echo "remote share not mounted. Check connection to server"
fi


