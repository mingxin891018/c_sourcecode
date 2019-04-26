#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define TIMEOUT_IN_MS 2

int main(int argc ,char *argv[])
{
	fd_set socket_set;
	struct timeval tv;
	unsigned char c;
	int ret = 0;
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	while(1)
	{
		printf("++++++++++++++++++++++\n");
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		FD_ZERO(&socket_set);
		FD_SET(0, &socket_set);

		ret = select(1,  &socket_set, NULL, NULL, &timeout);
		if(ret < 0)
		{	
			perror("select()");
			return -1;
		}	
		else if(ret == 0)
		{	
			perror("select()");
			continue;
		}
		else
			printf("can be read!\n");

		if(read(0,&c,sizeof(c)) < 0)
		{
			perror("read()");
			return -1;
		}
		printf("c=%d\n",c);
	}
	return 0;
}

