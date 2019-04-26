#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
	FILE *fp;
	
	fp = fopen("tmp","r+");
	if(fp == NULL)
	{
		perror("fopen()");
		exit(1);
	}

	fseek(fp,3,SEEK_SET);
	
	fputc('X',fp);

	rewind(fp);

	fputc('Y',fp);

	fclose(fp);	

	exit(0);
}


