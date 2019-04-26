#!/bin/sh

#while true
#do
#	getprop persist.sys.instructions.time >> /data/flash.txt
#	echo "============================"  >> /data/flash.txt
#	sleep 10
#	
#done

count=0
iseth0="empty"
iswlan0="empty"
while [ ${iseth0} = "empty" -a ${iswlan0} = "empty" ]
do
	if [ ${count} -eq 5 ] ;then
		break
	fi  

	echo "0000"
	sleep 1
	count=$[$count+1]
	iseth0="eth0"
done

if [ ${iseth0} = "eth0"	] ;then
	echo "111111111"
fi

if [ ${iswlan0} = "wlan0" ] ;then
	echo "222222222"
fi














