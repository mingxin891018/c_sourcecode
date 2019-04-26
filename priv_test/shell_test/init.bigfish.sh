#!/system/bin/sh
mount -o remount system /system
chmod 777 /etc/android.option.sh
chmod 777 /system/bin/setxattr
chmod 777 /system/bin/swlog.sh
setxattr /system/bin/run-as 0xc0
setxattr /system/bin/dhcpcd 0x3c00
mount -ro remount system /system

if [ -x /system/etc/swlog.sh ];then
	/system/etc/swlog.sh &
fi

date -s 20140101
QBENABLE=`getprop persist.sys.qb.enable`
case $QBENABLE in
 true)
  ;;
 *)
  setprop sys.insmod.ko 1
  ;;
 esac

#DVFS
echo 300000 > /sys/module/mali/parameters/mali_dvfs_max_freqency

#增加tcp滑动窗口大小和套接字缓存大小，增加tcp下载的并发数，从而增加下载速率
echo f > /sys/class/net/eth0/queues/rx-0/rps_cpus

mkdir /swdb/devinfo
chown system:system /swdb/devinfo
chmod 700 /swdb/devinfo
#Chmod For MTK Wi-Fi 
mkdir -p /data/Wireless/RT2870AP
chmod -R 777 /data/Wireless/RT2870STA
chmod -R 777 /data/Wireless/RT2870AP

echo "\n\nWelcome to HiAndroid\n\n" > /dev/kmsg
LOW_RAM=`getprop ro.config.low_ram`
case $LOW_RAM in
 true)
  echo "\n\nenter 512M low_ram mode\n\n" > /dev/kmsg
  echo 104857600 > /sys/block/zram0/disksize
  mkswap /dev/block/zram0
  swapon /dev/block/zram0
  #modules(memopt): Enable KSM in low ram device
  echo 1 > /sys/kernel/mm/ksm/run
  echo 300 > /sys/kernel/mm/ksm/sleep_millisecs
  ;;
 *)
  ;;
 esac

if [ ! -d /data/playready ]; then
    echo "copy playready cert"
    mkdir -p  /data/playready/
    cp -rfp  /system/etc/playready/*   /data/playready/
    chown  system:system  /data/playready
    chmod -R 777 /data/playready/
fi

if [ -f /cache/update/update.zip ]; then
  echo "removing update package"
  rm /cache/update/update.zip
else
  echo "update package already deleted"
fi

#if [ -f /cache/executeproducttest.txt ]; then
#  echo "open adbd set prop"
#  setprop persist.sys.usb.config adb
#  setprop persist.sys.sw.adb_root 1
#  setprop persist.sys.sw.console 1
#else
#  echo "not executeproducttest.txt"
#fi

if [ -f /cache/update/manufactoryupdate.flag ]; then
  echo "removing manufactoryupdate.flag"
  rm /cache/update/manufactoryupdate.flag
else
  echo "manufactoryupdate.flag already deleted"
fi

if [ ! -d /data/share/logkmsg ];then
         mkdir -p /data/share/logkmsg
fi
if [ ! -f /data/share/logkmsg/count ]; then
         echo 1 > /data/share/logkmsg/count
         str=1        
else
         str=`cat /data/share/logkmsg/count`
         let str+=1;
         echo $str > /data/share/logkmsg/count
fi

#resolve IP conflict when net and wifi connect the same arp.
echo 1 > /proc/sys/net/ipv4/conf/all/arp_ignore
Dei_status=$(getprop 'persist.sys.cus.video.status')
echo limit_framerate off >proc/msp/omxvdec
if [ "${Dei_status}" = "play" ];then
	echo set_Dei on >proc/msp/omxvdec
else
	echo set_Dei off >proc/msp/omxvdec
fi
echo limit_framerate on >proc/msp/omxvdec



/etc/android.option.sh

#for skype question:avoid Skype do not call the camera
filename="/data/data/com.skype.*/cache/camera_params.bin"
delete_flag=1
cat $filename|while read LINE
do
{
  if [[ "$LINE" = "*com.skype.android.platform.capture.CameraCapabilities*" ]];then
    delete_flag=0
   fi
}
done 
if [[ "$delete_flag" = "1" ]];then
  echo "delete file:$filename"
  rm $filename
fi

if [ "`ls -a /data/lost+found`" != "" ];then
        rm -r /data/lost+found/*
fi

if [ "`ls -a /cache/lost+found`" != "" ];then
        rm -r /cache/lost+found/*
fi
