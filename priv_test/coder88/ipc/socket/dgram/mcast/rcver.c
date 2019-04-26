#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "proto.h"

#define IPSTRSIZE	128

int main()
{
	struct sockaddr_in laddr,raddr;
	int sd;
	struct msg_st rbuf;
	socklen_t raddr_len;
	char ipstr[IPSTRSIZE];

	sd = socket(AF_INET,SOCK_DGRAM,0/*IPPROTO_UDP*/);
	if(sd < 0)
	{
		perror("socket()");
		exit(1);
	}

	struct ip_mreqn mreq;
    inet_pton(AF_INET,MGROUP,&mreq.imr_multiaddr);
    inet_pton(AF_INET,"0.0.0.0",&mreq.imr_address);
    mreq.imr_ifindex = if_nametoindex("eth0");
    if(setsockopt(sd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
    {
        perror("setsockopt()");
        exit(1);
    }

	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(atoi(RCVPORT));
	inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
	if(bind(sd,(void *)&laddr,sizeof(laddr)) < 0)
	{
		perror("bind()");
		exit(1);
	}

	raddr_len = sizeof(raddr);

	while(1)
	{
		if(recvfrom(sd,&rbuf,sizeof(rbuf),0,(void *)&raddr,&raddr_len) < 0)
		{
			perror("recvfrom()");
			exit(1);
		}
	
		inet_ntop(AF_INET,&raddr.sin_addr,ipstr,IPSTRSIZE);
		printf("---MESSAGE FROME:%s:%d---\n",ipstr,ntohs(raddr.sin_port));

		printf("NAME = %s\n",rbuf.name);
		printf("MATH = %d\n",ntohl(rbuf.math));
		printf("CHINESE = %d\n",ntohl(rbuf.chinese));
	}	
	close(sd);

	exit(0);
}


