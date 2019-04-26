#!/bin/sh

#该脚本可用于查询证书过期时间
#把要查询的证书放在一个路径下，进入该路径查询,过滤证书，查询有效期，比较当前时间，判断是否过期

export OUT=$PWD
echo "请输入要查询证书所在的路径：(如：./ /temp/)"
read -p "input path:" FilePath;
echo "FilePath = ${FilePath}"

#遍历指定路径下有效的证书
function getAllFiles()
{
	fileList=`ls $FilePath`	
	echo "请输入要查询的证书的后缀名: (如: crt)"
	read -p "请输入要查询的证书后缀，不带. : " SUFFIX_SRC;      

	for fileName in `find $fileList -name "*.${SUFFIX_SRC}"`;
	do
		if [ '-f' $fileName ];then
			#fl=$fileName
			Befor=$(grep "Not Before" $fileName ) 
			After=$(grep "Not After" $fileName )
			echo "Certificate: ${fileName} ${Befor} ${After}"

			expiredtime=`echo $After|awk -F ': ' '{print $2}'`
			expiredday=$(( ($(date -d "$expiredtime" +%s)-$todays)/86400 ))
			if [ ${expiredday} -le '0' ];then
				echo " ！！！ 证书已经过期 ！！！"
			else
				echo "距离证书 ${fileName}   过期还有 ${expiredday} 天!"
			fi
		else
			echo "$FilePath is a invalid path";
		fi
	done
}

cd $FilePath;	
echo "---------------当前系统时间-------------" 
todays=`date +%s`
echo "$(date +"%F %T")"
echo "---------------开始查询-------------" 
getAllFiles;
echo "---------------查询结束-------------" 
