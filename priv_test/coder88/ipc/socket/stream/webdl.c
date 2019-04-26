#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFSIZE		1024

int main(int argc,char **argv)
{
    int sd;
    struct sockaddr_in raddr;
	long long stamp;
	int ret;
	FILE *fp;
	char buf[BUFSIZE];
	int len;

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
	raddr.sin_port = htons(80);
	inet_pton(AF_INET,argv[1],&raddr.sin_addr);
	if(connect(sd,(void *)&raddr,sizeof(raddr)) < 0)
	{
		perror("connect()");
		exit(1);
	}

	fp = fdopen(sd,"r+");
	/*if error*/		

	fprintf(fp,"GET /test.jpg\r\n\r\n");
	fflush(fp);

	while(1)
	{
		len = fread(buf,1,BUFSIZE,fp);
		if(len <= 0)
			break;
		fwrite(buf,1,len,stdout);
	}

	fclose(fp);

	exit(0);

}

