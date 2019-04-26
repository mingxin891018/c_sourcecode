#!/bin/sh
##set -x
if [ $# -eq 0 ]
then
	echo "invalid params !!!"
	echo "input the name of orgin zip/apk, such as:"
	echo "./sign.sh disabl_serial.zip"
	echo "./sign.sh SWSetting.apk"
	exit
fi

#build_path="/home/zhaomingxin/small_system/iptv_tmsmall_6v5/build"
build_path="/home/zhaomingxin/small_system/tm_casmall_8v8/platform/build/tool"
input_file=$1
suffix_name=${input_file##*.}
prefix_name=${input_file%".$suffix_name"}
cmd_key="openssl pkcs8 -in $build_path/ota/security/testkey.pk8 -inform DER -nocrypt"
cmd_key_8v8="openssl pkcs8 -in $build_path/security/testkey.pk8 -inform DER -nocrypt"

if [ ${suffix_name} == "zip" ]
then
    output_file="$prefix_name"_sign.zip
    cmd="java -Xmx2048m -jar $build_path/linux-x86/framework/signapk.jar -w $build_path/ota/security/testkey.x509.pem $build_path/ota/security/testkey.pk8 $build_path/ota/security/testkey2.x509.pem $build_path/ota/security/testkey2.pk8  $input_file $output_file"
	cmd_8v8="java -Xmx2048m -jar $build_path/linux-x86/framework/signapk.jar -w $build_path/security/testkey.x509.pem $build_path/security/testkey.pk8 $input_file $output_file"

else if [ ${suffix_name} == "apk" ]
	then
    	output_file="$prefix_name"_sign.apk
    	cmd="java -jar $build_path/linux-x86/framework/signapk2.jar $build_path/ota/security/platform.x509.pem $build_path/ota/security/platform.pk8  $input_file $output_file"

	else
    	echo "Invalid param,need *.zip/*.apk "
    	exit
	fi
fi

echo "input : $input_file"
echo "output: $output_file"
echo $cmd_key_8v8
$cmd_key_8v8
echo $cmd_8v8
$cmd_8v8











