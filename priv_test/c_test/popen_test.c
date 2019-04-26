#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{
	char buf[1024] = {0};
	FILE *fp = popen("ps | grep mingxin", "r");
	fgets(buf,sizeof(buf),fp);
	printf("buf=%s\n", buf);
	fclose(fp);


	return 0;
}


