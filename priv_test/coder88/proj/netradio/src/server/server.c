#include <stdio.h>

/*
 *  -M  指定多播组
 *  -P  指定测试端口
 *  -C  指定测试频道
 *  -F	前台调试运行
 *  -H  显示帮助
 * */


int main(    ,   )
{
	struct mlib_chn_st *list;
	int listsize;

	/* conf的处理 */

	/*socket初始化*/

	/*获得频道列表*/
	ret = mlib_getchnlist(&list,&listsize);	
	if()
	{
		syslog();
	}


	/*创建thr_list*/
	thr_list_create(list,listsize);	



	/*创建thr_channel*/
	for(i = 0 ; i < listsize; i++)
		thr_chnnel_create();


	while(1)
		pause();

	exit(0);
}

