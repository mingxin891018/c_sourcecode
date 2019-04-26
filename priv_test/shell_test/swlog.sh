#!/system/bin/sh

DF="toolbox df";
AWK="busybox awk";
GREP="busybox grep";
HEAD="busybox head"
IFCONFIG="busybox ifconfig";
TCPDUMP="/system/xbin/tcpdump";
SLEEP="busybox sleep";
KILL="busybox kill";
ECHO="busybox echo";
SORT="busybox sort";
KILLALL="busybox killall";

PROCS="";

Debug() {
	${ECHO} $@;
}

FindMnt() {
	for i in $(${DF} | ${GREP} "/mnt/sd" | ${SORT} | ${AWK} -F ' ' '{print $1}');do
		if [ -d $i ];then
			if [ -w $i ];then
				${ECHO} "$i";
				break;
			fi
		fi
	done
}

CheckIfconfig() {
	${IFCONFIG} $1 >/dev/null 2>&1
	if [ $? == 0 ];then
		echo "true";
	else
		echo "false";
	fi
}

StartTcpdump() {
	while true;do
		if [ "$(CheckIfconfig $1)" == "true" ];then
			Debug "${TCPDUMP} -s0 -i $1 -w ${CACHE_DIR}/$1.cap"
			${TCPDUMP} -s0 -i $1 -w ${CACHE_DIR}/$1.cap
			Debug "Stop tcpdump $1 $!";
			break;
		fi
	done
}

DATE() {
	busybox date +%Y%m%d%H%M%S;
}


CACHE_DIR=/tmp/log/;
WORK_DIR="";

Debug "Start service";
echo "${DF} | ${GREP} "/mnt/sd" | ${SORT} | ${AWK} -F ' ' '{print \$1}'";
FindMnt;
Debug "Goto loop";
while true;do
	while true;do
		WORK_DIR=$(FindMnt);
		if [ "${WORK_DIR}" == "" ];then
			${SLEEP} 1;
		else
			Debug "Find munt ${WORK_DIR} to catch log";
			break;
		fi
	done
	# 初始化参数
	CACHE_DIR="${WORK_DIR}/log_$(DATE)";
	PROCS="";
	Debug mkdir -p ${CACHE_DIR};
	rm -rf ${CACHE_DIR};
	mkdir -p ${CACHE_DIR};
	busybox chmod 777 -R ${CACHE_DIR};
	# 开始工作
	logcat -vtime > ${CACHE_DIR}/logcat.txt&
	PROCS="${PROCS} $!"
	Debug "Start logcat ${PROCS}";
	StartTcpdump "eth0"&
	PROCS="${PROCS} $!"
	Debug "Start eth0 ${PROCS}";
	StartTcpdump "wlan0"&
	while true;do
		busybox date >> ${CACHE_DIR}/route.txt
		busybox route >> ${CACHE_DIR}/route.txt
		sleep 1
	done
	PROCS="${PROCS} $!"
	Debug "Start wlan0 ${PROCS}";
	while true;do
		ls ${CACHE_DIR} > /dev/null 2>&1;
		if [ $? == 0 ];then
			${SLEEP} 0.5;
		else
			${KILL} -9 ${PROCS};
			Debug "Stop ${PROCS}"
			Debug "${KILLALL} -9 ${TCPDUMP}";
			${KILLALL} -9 tcpdump logcat;
			break;
		fi
	done
done

