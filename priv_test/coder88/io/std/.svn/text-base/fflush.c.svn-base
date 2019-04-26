#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 *缓冲区的存在在大多数情况下都是好事，作用是：合并系统调用
 *
 * 行缓冲：满了的时候刷新，强制刷新，换行的时候刷新，（如stdout，因为涉及到了终端设备）
 *
 * 全缓冲：满了的时候刷新，强制刷新，（默认使用全缓冲模式，只要不是终端设备）
 *
 * 无缓冲：需要立刻输出，例：stderr
 *
 *
 * */


int main()
{
	int i;
	
	for(i = 0 ; i < 10 ;i ++)
	{
		putchar('*');
		fflush(NULL);
		sleep(1);
	}
		
	putchar('\n');



/*

	int i = 1,j = 3;

	printf("[%s:%d]Before while()",__FUNCTION__,__LINE__);
	fflush(stdout);

	while(j > 0);
	{
		i++;
		j--;
	}

	printf("[%s:%d]After while()",__FUNCTION__,__LINE__);
	fflush(NULL);

	printf("ok\n");
*/
	exit(0);
}


