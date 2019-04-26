#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>

#include "proto.h"

int main(int argc,char **argv)
{
    int sd;
    struct sockaddr_in raddr;
	long long stamp;
	int ret;
	FILE *fp;
	char timestr[1024];

	if(argc < 2)
	{
		perror("Usage...\n");
		exit(1);
	}

    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }

	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(atoi(SERVERPORT));
	inet_pton(AF_INET,argv[1],&raddr.sin_addr);
	if(connect(sd,(void *)&raddr,sizeof(raddr)) < 0)
	{
		perror("connect()");
		exit(1);
	}

	fp = fdopen(sd,"r+");
	/*if error*/		

	if(fscanf(fp,FMT_STAMP,&stamp) != 1)
		printf("failed.\n");
	else
		printf("stamp = %lld\n",stamp);

	fclose(fp);
#if 0

	//if((ret = recv(sd,&stamp,sizeof(stamp),0)) < 0)
	if((ret = recv(sd,timestr,1024,0)) < 0)
	{
		perror("recv()");
		exit(1);
	}
	printf("ret = %d\n",ret);

//	printf(FMT_STAMP,stamp);
	puts(timestr);
	close(sd);

#endif

	exit(0);

}


