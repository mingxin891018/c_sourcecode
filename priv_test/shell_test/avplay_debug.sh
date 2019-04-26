echo "**********avplay debug run*********" 
if [ $# -eq 0 ] ; then 
echo "USAGE: $0 /mnt/sda/sda1/avplsy_proc.log "
xxxx="/mnt/sda/sda1/avplsy_proc.log"
else
xxxx=$1

fi

rm -f $xxxx

logbug=/sdcard/logcat.log 
rm -f $logbug
logcat -c
logcat -v threadtime -f $logbug  &

echo "***********************************" 
echo "log path1: $xxxx "
echo "log path2: $logbug "
echo "***********************************"
 
echo "***********avplay debug 1.1************" > $xxxx
cat /proc/msp/sys   >> $xxxx ;

#echo 0 0x800003 >/proc/vfmw  
#echo 0 0 >/proc/vfmw  
while :
do
time=`date +'%Y-%m-%d %H:%M:%S'`;
echo ""                  >> $xxxx 
echo "********** $time ************" >> $xxxx ; 
echo ""                  >> $xxxx 
cat /proc/hisi/*        >> $xxxx ;
cat /proc/vfmw_dec      >> $xxxx ;
cat /proc/vfmw_scd      >> $xxxx ;
cat /proc/msp/vdec*     >> $xxxx ;
cat /proc/msp/vpss*     >> $xxxx ;
cat /proc/msp/win*      >> $xxxx ;
cat /proc/msp/avplay*   >> $xxxx ;
cat /proc/msp/omxvdec   >> $xxxx ;
cat /proc/msp/demux_*	  >> $xxxx ;
cat /proc/msp/syn*      >> $xxxx ;
cat /proc/msp/adec*     >> $xxxx ;
cat /proc/msp/sound*    >> $xxxx ;
cat /proc/msp/disp*     >> $xxxx ;
cat /proc/msp/pm_cpu    >> $xxxx ;
cat /proc/media-mem     >> $xxxx ;
cat /proc/msp/hdmi*     >> $xxxx ;
cat /proc/interrupts    >> $xxxx ;  

sleep 1;              
echo ''
done

chmod -R 777 $xxxx
