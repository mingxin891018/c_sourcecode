#! /bin/bash

function main_menu {

clear
echo
echo  -e "\t\t\t 主菜单: "
echo  -e "\t1,"
echo  -e "\t2,"
echo  -e "\t3,"
echo  -e "\t0,退出:\n\n "

echo  -en "\t\t现在输入选项: "
read -n 1 option

}

function diskspace {
	echo -e "选择1。"
}

function whoseon {
	echo -e "选择2。"

}

function menusage {
	echo -e "选择3。"
}

main_menu

case $option in 
0)
	break ;;
1)
	diskspace ;;
2)
	whoseon ;;
3)
	menusage ;;
*)
	clear 
	echo "错误的选项！！！" ;;
esac

