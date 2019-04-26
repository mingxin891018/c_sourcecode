#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <proto.h>
#include <share.h>

static struct fsm_st *fsm;

//上传文件时的状态机
int fsm_up(struct fsm_st *fsm)
{
	if(fsm == NULL)
	{
		printf("fsm_up():error!\n");
		return -1;
	}
	switch(fsm->state)
	{
		case SND_PATH:
			send_up_path(fsm);
			break;
		case READ_FILE:
			read_up_file(fsm);
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
			printf("找不到文件,上传失败\n");
			break;
		default:
			break;
	}
	return 0;
}

//下载文件时的状态机
int fsm_down(struct fsm_st *fsm)
{
	if(fsm == NULL)
	{
		printf("fsm_up():error!\n");
		return -1;
	}
	switch(fsm->state)	
	{
		case SND_PATH:
			send_path(fsm);
			break;
		case SEND_ACK:
			send_ack(fsm);
			break;
		case RCV_DATA:
			receive_data(fsm);
			break;
		case RCV_LENGTH:
			receive_length(fsm);
			break;
		case RCV_EOT:
			break;
		case Ex:
			perror(fsm->errname);
			break;
		case T:
			printf("找不到文件,下载失败\n");
			break;
		default:
			break;
	}
	return 0;
}

//取消下载时的信号处理函数
void load_cancle(int sig,siginfo_t *siginfo,void *data)
{
	int ret = 0,count = 50;
	long res_up = 0,res_down = 0;

	res_down = (fsm->file_down*100) / fsm->file_length;
	res_up = (fsm->file_up*100) / fsm->file_length;

	if((res_up > 90) || (res_down > 90))
	{
		return ;
	}
	while(count--)
	{
		memset(fsm->buf,'\0',fsm->size);
		fsm->buf->flag = 'c';
		ret = sendto(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,fsm->raddr_len);
		if(ret < 0)
		{
			perror("sendto()");
			printf("取消操作失败\n");
		}
	}
	exit(0);
}

int main(int argc,char * argv[])
{
	int err = 0,ch = 0,flag = 0;
	struct sigaction sa;
	char ip[NAMESIZE] = {'\0'};
	char path[NAMESIZE] = {'\0'};
	char *p;
	if(argc != 3)
	{
		fprintf(stderr,"Usage:<./> <ip:path> <mode:-u上传 -d下载>\n");
		exit(1);
	}
	while(1)
	{
		ch = getopt(argc,argv,"-ud");
		if(ch < 0)
			break;
		switch(ch)
		{
			case 1:
				strncpy(ip,argv[optind-1],NAMESIZE-1);
				break;
			case 'u':
				flag = 2;
				break;
			case 'd':
				flag = 1;
				break;
		}
	}
	if(strlen(argv[optind - 1]) >NAMESIZE)
		printf("参数过长\n");
	p = strchr(ip,':');
	if(p == NULL)
	{
		printf("下载地址错误格式IP:path\n");
		return -1;
	}
	*p = '\0';
	strncpy(path,p+1,strlen(p+1));

	fsm = malloc(sizeof(*fsm));
	if(fsm == NULL)
	{
		perror("malloc()");
		return -1;
	}
	fsm->size =DATEMAX;
	fsm->buf = malloc(fsm->size);
	if(fsm->buf == NULL)
	{
		free(fsm);
		perror("malloc()");
		goto error;
	}
	memset(fsm->buf,'\0',sizeof(struct msg_st));
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

	fsm->raddr.sin_family = AF_INET;
	fsm->raddr.sin_port = htons(atoi(RCVPORT));
	if(inet_pton(AF_INET,ip,&fsm->laddr.sin_addr) != 1)
	{
		perror("inet_pton()");
		goto error;
	}
	fsm->raddr_len = sizeof(fsm->raddr);

	strncpy((char*)fsm->buf->data,path,strlen(path));
	strncpy(fsm->pathname,path,strlen(path));
	strncpy(fsm->upload_pathname,path,strlen(path));

	sa.sa_sigaction = load_cancle;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask,SIGQUIT);
	sigaddset(&sa.sa_mask,SIGINT);

	sa.sa_flags = 1;
	err = sigaction(SIGINT,&sa,NULL);
	if(err < 0)
		perror("sigaction()");

	fsm->state = SND_PATH;
	if(flag == 1)
	{	
		fsm->buf->flag = 'p';
		while(fsm->state < STATE_AUTO)
		{	
			if(fsm_down(fsm) < 0)
			{
				printf("fsm_down():is error\n");
				goto error;
			}
		}
		if(fsm_down(fsm) < 0)
		{
			printf("fsm_down():is error\n");
			goto error;
		}
	}
	else if(flag == 2)
	{	
		fsm->buf->flag = 'n';
		while(fsm->state < STATE_AUTO)
		{
			if(fsm_up(fsm) < 0)
			{
				printf("fsm_up():is error\n");
				goto error;
			}
		}
		if(fsm_up(fsm) < 0)
		{
			printf("fsm_up():is error\n");
			goto error;
		}
	}
	else 
	{	
		printf("参数错误\n");
		goto error;
	}	

	free(fsm->buf);
	fsm->buf = NULL;
	close(fsm->sd);
	close(fsm->fd);
	pthread_join(fsm->tid,NULL);
	pthread_join(fsm->tidres,NULL);
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


