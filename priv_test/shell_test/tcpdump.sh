#!/system/bin/sh

function rm_file()
{
	if [ -f ${1} ];then
		rm -rf ${1}
	fi
}

file="/data/log.txt"
baowen1="/data/eth0.cap"
baowen2="/data/wifi.cap"

rm_file ${file}
rm_file ${baowen1}
rm_file ${baowen2}

/system/bin/logcat -vtime > ${file} &
count=0
iseth0="empty"
iswlan0="empty"
while [ ${iseth0} = "empty" -a ${iswlan0} = "empty" ]
do
	iseth0=$(busybox ifconfig  | busybox awk -F ' ' '$1=="eth0" {print $1}' | busybox awk '{print $1}')
	iswlan0=$(busybox ifconfig  | busybox awk -F ' ' '$1=="wlan0" {print $1}' | busybox awk '{print $1}')
	count=$[$count+1]
	if [ -z ${iseth0} ] ;then
		iseth0="empty"
	fi
	if [ -z ${iswlan0} ] ;then
		iswlan0="empty"
	fi
	sleep 1
done

echo ${iseth0}
echo ${iswlan0}

function do_tcpdump()
{
ret="wrong"
if [ ${iseth0} = $1 ];then
	while [ ${ret} = "wrong" ]
	do
		/system/xbin/tcpdump -i $1  -s0 -w /data/$1.cap &
		res=$(busybox ps | busybox grep "tcpdump")
		if [ -z ${ret} ] ;then
			ret="wrong"
		else 
			echo "tcpdump eth0 is ok!!!!"
		fi
	
	done
fi
}

if [ ${iseth0} = "eth0" ];then
	do_tcpdump ${iseth0}
fi
	
if [ ${iseth0} = "wlan0" ];then
	do_tcpdump ${wlan0}
fi



