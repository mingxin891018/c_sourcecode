/*
	测试申请的内存中的初始值是否为0
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


//static char buf[24];
int main(int agrc,char *argv[])
{
	
	char buf[24] ;
	uint32_t a=1;
	uint32_t b=1;
	uint32_t c=1;
	uint32_t d=1;
	uint32_t e=1;
	uint32_t f=1;
	
//	memset(buf,0,sizeof(buf));
	memcpy(&a,buf,4);
	memcpy(&b,buf+4,4);
	memcpy(&c,buf+8,4);
	memcpy(&d,buf+12,4);
	memcpy(&e,buf+16,4);
	memcpy(&f,buf+20,4);
	
	printf("%d %d %d %d %d %d\n",a,b,c,d,e,f);

	return 0;
}



