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

//下载进度线程函数
void *pthread_download(void *arge)
{
	long length = 0,finish = 0;
	if(arge == NULL)
	{
		printf("arguments is error!\n");
		return NULL;
	}
	struct fsm_st *fsm = arge;
	printf("下载进度: ");
	fflush(stdout);
	while(fsm->running)
	{
		length = fsm->file_down;	
		finish = (length*100)/fsm->file_length;
		printf("%ld%% ",finish);
		fflush(stdout);
		if(finish == 100)
			fsm->running = 0;
		sleep(1);
	}
	printf("下载完成\n");
	return NULL;
}
//上传进度线程函数
void *pthread_upload(void *arge)
{
	int ret = 0,finish = 0;
	off_t file_length = 0;
	long length = 0;
	struct stat statres;
	if(arge == NULL)
	{
		printf("arguments is error!\n");
		return NULL;
	}
	struct fsm_st *fsm = arge;
	ret = lstat(fsm->pathname,&statres);
	if(ret < 0)
	{
		perror("lstat()");
		return NULL;
	}

	printf("上传进度: ");
	fflush(stdout);
	file_length = statres.st_size;
	while(fsm->now)
	{
		length = ftell(fsm-> fp1);
		finish = (length*100)/file_length;

		printf("%d%% ",finish);
		fflush(stdout);
		if(finish >= 100)
			fsm->now = 0;
		sleep(1);
	}
	printf("上传完成\n");
	return NULL;
}

unsigned long _receive_data(struct fsm_st *fsm,int fd)
{
	unsigned long length = 0;
	int ret = 0,len = 0,cur = 0;
	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_BLOCK,&set,NULL);
	ret = recvfrom(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,&fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"recvfrom()",strlen("recvform()"));
		fsm->state = Ex;
	}
	else
	{
		if(fsm->buf->flag == 'd')
		{
			len = ntohl(fsm->buf->len);
			while(len > 0)
			{
				ret = write(fd,fsm->buf->data,len + cur);
				if(ret < 0)
				{
					strncpy(fsm->errname,"recvfrom()",strlen("recvform()"));
					fsm->state = Ex;
				}
				len -= ret;
				cur += ret;
			}
			fsm->buf->flag = 'a';
			fsm->state = SEND_ACK;
		}
		else if(fsm->buf->flag == 'e')
		{	
			fsm->state = RCV_EOT;
			close(fd);
		}
		else if(fsm->buf->flag == 'c')
			fsm->state = RCV_EOT;
		length = ntohl(fsm->buf->len);
	}
	sigprocmask(SIG_SETMASK,&set,NULL);
	return length;
}

void receive_data(struct fsm_st *fsm)
{
	fsm->file_down += _receive_data(fsm,fsm->fd);
}

void receive_up_data(struct fsm_st *fsm)
{
	int fd = fileno(fsm->fp);
	if(fd < 0)
	{
		perror("fileno()");
		exit(0);
	}
	fsm->file_up += _receive_data(fsm,fd);
}

void receive_length(struct fsm_st *fsm)
{
	int ret = 0,err = 0;
	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_BLOCK,&set,NULL);
	ret = recvfrom(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,&fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"recvfrom()",strlen("recvform()"));
		fsm->state = Ex;
	}
	else
	{
		if(fsm->buf->flag == 'l')
		{
			memcpy(&fsm->file_length,fsm->buf->data,ntohl(fsm->buf->len));
			printf("文件长度是:%ld\n",fsm->file_length);
			fsm->state = RCV_DATA;
			err = pthread_create(&fsm->tid,NULL,pthread_download,fsm);
			if(err)
				perror("pthread_create()");
		}
		else if(fsm->buf->flag == 'e')
			fsm->state = RCV_EOT;
		else if(fsm->buf->flag == 'r')
			fsm->state = T;
		else if(fsm->buf->flag == 'c')
			fsm->state = RCV_EOT;
		else 
		{
			printf("接收文件长度失败!\n");
			fsm->state = T;
		}
	}
	sigprocmask(SIG_SETMASK,&set,NULL);
}

void _send_path(struct fsm_st *fsm,int state)
{
	int ret = 0;
	char *p = NULL,*q = NULL;
	char buf[NAMESIZE]={'\0'};
	ret = sendto(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"sendto()",strlen("sendto()"));
		fsm->state = Ex;
	}
	else
	{
		if(state == RCV_LENGTH)
		{
			getcwd(buf,NAMESIZE);
			p = strrchr(buf,'/');
			if(p == NULL)
			{	
				fsm->state = Ex;
				return ;
			}
			*p = '\0';
			p = strrchr(buf,'/');
			strncat(p+1,"/down_load/",12);
			q = strrchr(fsm->pathname,'/');
			if(q == NULL)
			{	
				fsm->state = Ex;
				return ;
			}
			strncat(buf,q+1,strlen(q+1));
			fsm->fd = open(buf,O_RDWR|O_CREAT|O_TRUNC,0777);
			if(fsm->fd < 0 )
			{	
				strncpy(fsm->errname,"open()",strlen("open()"));
				fsm->state = Ex;
			}
			else
				fsm->state = state;
		}
		else if(state == READ_FILE)
		{	
			fsm->fp1 = fopen((char*)fsm->buf->data,"r");
			fseek(fsm->fp1,0,SEEK_END);
			fsm->file_length = ftell(fsm->fp1);
			rewind(fsm->fp1);
			if(fsm->fp1 == NULL)
			{
				strncpy(fsm->errname,"fopen()",sizeof("fopen()"));
				fsm->state = Ex;
			}
			else
				fsm->state = state;
		}
	}
}

void send_path(struct fsm_st *fsm)
{
	_send_path(fsm,RCV_LENGTH);
}

void send_up_path(struct fsm_st *fsm)
{
	int ret = 0;
	_send_path(fsm,READ_FILE);
	ret = pthread_create(&fsm->tidres,NULL,pthread_upload,fsm);
	if(ret)
		perror("pthread_create()");
}

void open_file(struct fsm_st *fsm)
{
		fsm->fp = fopen((char *)fsm->buf->data,"r");
		if(fsm->fp == NULL)
		{
			strncpy(fsm->errname,"fopen()",sizeof("fopen()"));
			fsm->state = Ex;
		}
		else 
			fsm->state = SEND_LENGTH;
}

void open_up_file(struct fsm_st *fsm)
{
	char *p = NULL,*q = NULL;
	char buf[NAMESIZE]={'\0'};
	p = strrchr((char*)(fsm->buf->data),'/');
	if(p == NULL)
	{
		printf("open_up_file():error!\n");
		fsm->state = Ex;
	}
	else
	{
		getcwd(buf,NAMESIZE);
		q = strrchr(buf,'/');
		if(q == NULL)
		{
			printf("open_up_file():error!\n");
			fsm->state = Ex;
		}
		*(q+1) = '\0';
		strncat(q+1,"up_load/",strlen("up_load/"));
		strncat(buf,p+1,strlen(p+1));
		fsm->fp = fopen(buf,"w");
	}
	if(fsm->fp == NULL)
	{
		strncpy(fsm->errname,"fopen()",sizeof("fopen()"));
		fsm->state = Ex;
	}
	else 
		fsm->state = RCV_DATA;
}

void send_ack(struct fsm_st *fsm)
{
	int ret = 0;
	ret = sendto(fsm->sd,fsm->buf,1,0,(void*)&fsm->raddr,fsm->raddr_len);
	if(ret < 0)
	{
		printf("send_ack\n");
		strncpy(fsm->errname,"sendto()",strlen("sendto"));
		fsm->state = Ex;
	}
	else
	{
		fsm->buf->flag = 'a';
		fsm->state = RCV_DATA;
	}
}

void receive_path(struct fsm_st *fsm)
{
	int ret = 0;
	printf("等待接收数据:\n");
	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_BLOCK,&set,NULL);
	ret = recvfrom(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,&fsm->raddr_len);
	if(ret < 0)
	{
		perror("recvfrom()");
		exit(1);
	}
	else
	{
		inet_ntop(AF_INET,&fsm->raddr.sin_addr,fsm->ipstr,IPSTRSIZE);
		printf("---消息来自:%s:%d---\n",fsm->ipstr,ntohs(fsm->raddr.sin_port));
	}	
	sigprocmask(SIG_SETMASK,&set,NULL);
}

ssize_t _read_file(struct fsm_st *fsm,FILE *fp)
{
	fsm->buf->len = htonl(fread(fsm->buf->data,1,TEST,fp));
	if(fsm->buf->len < TEST)
	{	
		if(ferror(fp))
		{	
			strncpy(fsm->errname,"fread()",strlen("fread()"));
			fsm->state = Ex;
		}
		else
		{
			fsm->buf->flag = 'e';
			fsm->state = SEND_EOT;
		}
	}
	else
		fsm->state = SEND_FILE;
	return ntohl(fsm->buf->len);
}

void read_up_file(struct fsm_st *fsm)
{
	ssize_t ret = 0;
	ret = _read_file(fsm,fsm->fp1);
	
	fsm->file_up += ret;
}

void read_file(struct fsm_st *fsm)
{
	_read_file(fsm,fsm->fp);
}

void send_file(struct fsm_st *fsm)
{
	int ret = 0;
	fsm->buf->flag = 'd';
	ret = sendto(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"sendto()",strlen("sendto()"));
		fsm->state = Ex;
	}
	else
		fsm->state = RCV_ACK;
}

void send_length(struct fsm_st *fsm)
{
	int ret = 0;
	long length = 0;
	fseek(fsm->fp,0,SEEK_END);
	length = ftell(fsm->fp);
	rewind(fsm->fp);

	memcpy(fsm->buf->data,&length,sizeof(long));
	fsm->buf->len = htonl(sizeof(long));
	fsm->buf->flag = 'l';

	ret = sendto(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"sendto()",strlen("sendto()"));
		fsm->state = Ex;
	}
	else
		fsm->state = READ_FILE;
}

void receive_ack(struct fsm_st *fsm)
{
	int ret = 0;
	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_BLOCK,&set,NULL);
	ret = recvfrom(fsm->sd,fsm->buf,fsm->size,0,(void*)&fsm->raddr,&fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"recvfrom()",strlen("recvform()"));
		fsm->state = Ex;
	}
	else 
	{
		if(fsm->buf->flag == 'a')
			fsm->state = READ_FILE;
		else if(fsm->buf->flag == 'c')
			fsm->state = SEND_EOT;
		else 
			fsm->state = SEND_FILE;
	}
	sigprocmask(SIG_SETMASK,&set,NULL);
}

void send_eof(struct fsm_st *fsm)
{
	int ret = 0;
	fsm->buf->flag = 'e';
	ret = sendto(fsm->sd,fsm->buf,1,0,(void*)&fsm->raddr,fsm->raddr_len);
	if(ret < 0)
	{
		strncpy(fsm->errname,"sendto()",strlen("sendto()"));
		fsm->state = Ex;
	}
	else
		fsm->state = STATE_AUTO;
}


