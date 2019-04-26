#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>          
#include <sys/socket.h>
/*
struct sockaddr_un {
	sa_family_t sun_family;               // AF_UNIX 
	char        sun_path[UNIX_PATH_MAX];  // pathname 
};
*/


#define CLOUD_PATH "cloud.tmp"
//#define CLOUD_PATH "cloud.tmp"
//#define CLOUD_PATH "/home/zhaomingxin/private/c_test/cloud.tmp"
#define LOG printf
static int sock_client = -1;
static int sock_connect = -1;


int main(int argc, char *argv[])
{

	char buf[512];
	memset(buf,0,sizeof(buf));
	cteate_socket_cloud();
	connect_socket_cloud();
	recv_data_cloud(buf,sizeof(buf));

	return 0;
}

int cteate_socket_cloud()
{

	sock_client = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock_client < 0)
	{
		LOG("[client]sock_client create filed\n");
		return -1;
	}
	LOG("[client]socket_cloud is %d\n",sock_client);
	
	return sock_client;
}

int connect_socket_cloud()
{
	struct sockaddr_un connect_addr;
	int addr_len = sizeof(struct sockaddr_un);
	
	memset(&connect_addr, 0, sizeof(struct sockaddr_un));
	connect_addr.sun_family = AF_UNIX;
	strncpy(connect_addr.sun_path,CLOUD_PATH , strlen(CLOUD_PATH));
	LOG("[client]client connect_addr.sunpath = %s\n",connect_addr.sun_path);
	
	if(sock_client < 0)
	{
		LOG("[client]sock_client is error\n");
		return -1;
	}
	sock_connect = connect(sock_client, (struct sockaddr *)&connect_addr, (socklen_t)addr_len);
	if(sock_connect < 0)
	{
		LOG("[client]connect is filed\n");
		perror("connect()");
		return -1;
	}
	
	return sock_connect;
}

int recv_data_cloud(char *buf, int len)
{
	int ret = 0;
	if((buf == NULL)|| (len <= 0) || (sock_connect < 0) )
	{
		LOG("[client]buf is error\n");
		return -1;
	}
	while(1){
		ret = send(sock_connect, "1111", 4,0);
		if(ret < 0)
		{
			LOG("[client]recv data filed\n");
			perror("recv()");
			return -1;
		}
		LOG("[client]read ret=%d\n",ret);
		LOG("[client]read data:%s",buf);
		sleep(1);
		printf("leep 1s\n");
	}
	close(sock_client);
	close(sock_connect);

}

