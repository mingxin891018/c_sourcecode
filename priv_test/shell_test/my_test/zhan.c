#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
	char buf[6*1024*1024];

	sprintf(buf,"%s\n",argv[1]);
	puts(buf);
	printf("\"");
	printf("%%");
	printf("#");
	return 0;
}

