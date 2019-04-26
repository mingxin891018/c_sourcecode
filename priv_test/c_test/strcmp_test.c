#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char *argv[])
{
	int ret = 0;
	char *p1 = "2017-11-27T11:27:14";
	char *p2 = "2017-11-27T11:25:00";
	ret = strcmp(p1,p2);
	printf("ret = %d\n",ret);



	return 0;
}

