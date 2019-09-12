#!/bin/sh
base_path=`pwd`
test_file_path="/tmp/sdcard/test.txt"
block_path="/dev/mmcblk0p1"
mount_path="/tmp/sdcard"
name="/tmp/udisk1"
df_name="$base_path/df.txt"
df > ${df_name}

function Read_Value
{
	local input_file=$1
	local val_name=$2

	local value=$(awk -v val=$val_name '$1==val {print $1,$6}' $input_file | awk '{print $2}')
	echo ${value}
}

function umount_sdcard
{
	value=$(Read_Value ${df_name} ${block_path})

	if [ -n ${value} ];then
		umount ${value}
	fi
}

function test_sdcard
{
	echo "===============SDCARD TEST=================="
	if [ ! -d ${mount_path} ];then
		busybox mkdir ${mount_path}
		echo "mkdir ${mount_path}"
	fi

	while [ true ]
	do
		if [ -b "${block_path}" ];then
			umount_sdcard
			mount -t vfat ${block_path} ${mount_path}
			echo "=======mount df info==============="
			df
			echo "=======mount df info==============="
			date=`date`
			if [ ! -f ${test_file_path}  ];then
				busybox touch ${test_file_path}
				echo "touch ${test_file_path}"
			fi

			echo "${date}"
			echo "${date}" >> ${test_file_path}

			umount ${mount_path}
			echo "=======unmount df info==============="
			df
			echo "=======unmount df info==============="
		else
			echo "/dev/mmcblk0p1 not exist"
		fi
		sleep 1
	done
}

test_sdcard




