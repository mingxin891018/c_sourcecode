#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <pthread.h>
#include <proto.h>
#include <share.h>

//下载文件时的状态机
int fsm_down(struct fsm_st *fsm)
{
	if(fsm == NULL)
	{
		printf("fsm_down():error!\n");
		return -1;
	}
	switch(fsm->state)
	{
		case RCV_PATH:
			receive_path(fsm);
			break;
		case OPEN_FILE:
			open_file(fsm);
			break;
		case READ_FILE:
			read_file(fsm);
			break;
		case SEND_LENGTH:
			send_length(fsm);
			break;
		case SEND_FILE:
			send_file(fsm);
			break;
		case RCV_ACK:
			receive_ack(fsm);
			break;
		case SEND_EOT:
			send_eof(fsm);
			break;
		case Ex:
			send_eof(fsm);
			perror(fsm->errname);
			break;
		case T:
			break;
		default:
			break;
	}
	return 0;
}

//上传时的状态机
int fsm_up(struct fsm_st *fsm)
{
	if(fsm == NULL)
	{
		printf("fsm_up():error!\n");
		return -1;
	}
	switch (fsm->state)
	{
		case OPEN_FILE:
			open_up_file(fsm);
			break;
		case SEND_ACK:
			send_ack(fsm);
			break;
		case RCV_DATA:
			receive_up_data(fsm);
			break;
		case RCV_EOT:
			break;
		case Ex:
			perror(fsm->errname);
			break;
		case T:
			break;
	}
	return 0;
}

int main(int argc,char * argv[])
{
	static struct fsm_st *fsm = NULL;
	fsm = malloc(sizeof(*fsm));
	if(fsm == NULL)
	{
		perror("malloc()");
		return -1;
	}
	memset(fsm,0,sizeof(*fsm));
	fsm->size = DATEMAX;
	fsm->buf = malloc(fsm->size);
	if(fsm->buf == NULL)
	{
		perror("malloc()");
		free(fsm);
		return -1;
	}
	memset(fsm->buf,0,fsm->size);
	
	fsm->now = 1;
	fsm->running = 1;
	fsm->file_length = 0;
	fsm->file_down = 0;
	fsm->file_up = 0;
	fsm->sd = socket(AF_INET,SOCK_DGRAM,0);
	if(fsm->sd < 0)
	{
		perror("socket()");
		goto error;
	}
	
	fsm->laddr.sin_family = AF_INET;
	fsm->laddr.sin_port = htons(atoi(RCVPORT));
	inet_pton(AF_INET,"0.0.0.0",&fsm->laddr.sin_addr);
	if(bind(fsm->sd,(void*)&fsm->laddr,sizeof(fsm->laddr)) < 0)
	{
		perror("bind()");
		goto error;
	}
	fsm->raddr_len = sizeof(fsm->raddr);

	while(1)
	{
		receive_path(fsm);
		if(fsm->buf->flag == 'p')
		{	
			printf("接收到要下载的路径名:\n");
			printf("%s\n",fsm->buf->data);
			fsm->state = OPEN_FILE;
			while(fsm->state < STATE_AUTO)
			{	
				if(fsm_down(fsm) < 0)
					goto error;
			}
			if(fsm_down(fsm) < 0)
				goto error;
		}
		else if(fsm->buf->flag == 'n')
		{
			printf("接收到要上传的文件名:\n");
			printf("%s\n",fsm->buf->data);
			fsm->state = OPEN_FILE;
			while(fsm->state < STATE_AUTO)	
			{	
				if(fsm_up(fsm) < 0)
					goto error;
			}
			if(fsm_up(fsm) < 0)
				goto error;
		}
		else 
			printf("收到未知数据类型包\n");
			continue;
	}
	free(fsm->buf);
	fsm->buf = NULL;
	free(fsm);
	fsm = NULL;
	return 0;
error:
	free(fsm->buf);
	fsm->buf = NULL;
	free(fsm);
	fsm = NULL;
	return -1;
}


