#!/usr/bin/env bash

export OUT=$PWD	
echo "重命名当前目录下所有文件名"
#echo "改良版，优化了交互，可以自定义需要更改的后缀名"
echo "请输入要替换的文件后缀名:"
read SUFFIX_SRC
echo "请输入要更改的文件后缀名:"
read SUFFIX_DST

#for i in $(ls $OUT |grep .$SUFFIX_SRC)
for i in *.$SUFFIX_SRC
do
if [ -e $i ]; then
#	echo "mv $i to `basename $i .$SUFFIX_SRC`.$SUFFIX_DST"
	mv $i `basename $i .$SUFFIX_SRC`.$SUFFIX_DST
else
	echo "file does not exist."
exit -1
fi														 
done
echo "所有文件后缀名已修改成功！"

:<<!
echo "基础版：剪切后缀名，然后重命名+新的后缀名"
oldext="crt"
newext="0"
for file in $(ls $OUT | grep .$oldext)
do
name=$(ls $file | cut -d. -f1)
mv $file ${name}.$newext
done
echo "change 0=====>crt done!"
!







