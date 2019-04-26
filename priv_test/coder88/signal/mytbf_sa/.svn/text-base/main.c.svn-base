#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "mytbf.h"

#define CPS			10
#define BUFSIZE		1024	
#define BURST		100

int main(int argc,char **argv)
{
	int sfd,dfd = 1;
	char buf[BUFSIZE];
	int len,ret,pos;
	mytbf_t *tbf;
	int size;

	if(argc < 2)
	{
		fprintf(stderr,"Usage:%s <srcfile> <destfile>\n",argv[0]);
		exit(1);
	}

	tbf = mytbf_init(CPS,BURST);
	if(tbf == NULL)
	{
		fprintf(stderr,"mytbf_init() failed.");
		exit(1);
	}


	do
	{
		sfd = open(argv[1],O_RDONLY);
		if(sfd < 0)
		{
			if(errno != EINTR)
			{
				perror("open()");
				exit(1);
			}
		}
	}while(sfd < 0);

	while(1)
	{

		size = mytbf_fetchtoken(tbf,BUFSIZE);
		if(size < 0)
		{
			fprintf(stderr,"mytbf_fetchtoken():%s\n",strerror(-size));
			exit(1);
		}


		while((len = read(sfd,buf,size)) < 0)
		{
			if(errno == EINTR)
				continue;
			perror("read()");
			exit(1);	
		}
		if(len == 0)
			break;
	
		if(size-len > 0)
			mytbf_returntoken(tbf,size-len);
		

		// len > 0
		pos = 0;		

		while(len > 0)
		{
			ret = write(dfd,buf+pos,len);
			if(ret < 0)
			{
				if(errno == EINTR)
					continue;
				perror("write()");
				exit(1);
			}
			len -= ret;
			pos += ret;
		}

	}

	close(sfd);

	mytbf_destroy(tbf);

	exit(0);
}


