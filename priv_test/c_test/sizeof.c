#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tmp {
	char name[8];
	char value[8];
} ;


int main(int argc,char *grgv[])
{
	struct tmp test[4];
	char buf[32]={0};
	char str[32] = {0};
	printf("sizeof(buf[32]) = %ld\n",sizeof(buf));
	printf("sizeof(struct tmp test[4]) = %ld\n",sizeof(test));
	
	chancan(1,str);
	return 0;
}

int chancan(int avc, char str[32])
{
	printf("sizeof(char str[32]) in chancan = %d\n",sizeof(str));

}


