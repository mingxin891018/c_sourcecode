#ifndef SHARE_H__
#define SHARE_H__
#include <proto.h>

#define IPSTRSIZE   128
#define NAMESIZE	1024

//上传下载状态机描述结构体
struct fsm_st
{
	int size;
	int now,running;
	int state,sd,fd;
	unsigned long file_length,file_down,file_up;

	char errname[ERRSIZE];
	char pathname[ERRSIZE];
	char upload_pathname[ERRSIZE];
	
	pthread_t tid,tidres;
	FILE *fp,*fp1;
	struct sockaddr_in laddr,raddr;
	socklen_t raddr_len;
	char ipstr[IPSTRSIZE];
	struct msg_st *buf;
};

//状态枚举
enum
{
	SND_PATH=1,
	RCV_PATH,
	OPEN_FILE,
	READ_FILE,
	
	SEND_LENGTH,
	RCV_LENGTH,
	
	SEND_FILE,
	RCV_DATA,
	
	RCV_ACK,
	SEND_ACK,

	SEND_EOT,
	STATE_AUTO,
	RCV_EOT,
	Ex,
	T
};
//发送下载路径
void send_path(struct fsm_st *fsm);

//上传文件名
void send_up_path(struct fsm_st *fsm);

//发送应答
void send_ack(struct fsm_st *fsm);

//发送文件数据
void send_file(struct fsm_st *fsm);

//发送文件长度
void send_length(struct fsm_st *fsm);

//发送文件尾标志
void send_eof(struct fsm_st *fsm);

//打开下载文件
void open_file(struct fsm_st *fsm);

//打开上传文件
void open_up_file(struct fsm_st *fsm);

//读下载文件
void read_file(struct fsm_st *fsm);

//读上传文件
void read_up_file(struct fsm_st *fsm);

//接收上传文件名
void receive_path(struct fsm_st *fsm);

//接收下载文件数据
void receive_data(struct fsm_st *fsm);

//接收上传文件数据
void receive_up_data(struct fsm_st *fsm);

//接收下载文件的长度
void receive_length(struct fsm_st *fsm);

//接收应答
void receive_ack(struct fsm_st *fsm);

#endif

