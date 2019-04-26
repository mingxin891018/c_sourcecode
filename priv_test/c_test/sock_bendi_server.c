#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <stddef.h>
/*
struct sockaddr_un {
	sa_family_t sun_family;               // AF_UNIX 
	char        sun_path[UNIX_PATH_MAX];  // pathname 
};
*/


//#define CLOUD_PATH "cloud.tmp"
#define CLOUD_PATH "cloud.tmp"
//#define CLOUD_PATH "/home/zhaomingxin/private/c_test/cloud.tmp"
#define LOG printf
static int sock_server = -1;
static int sock_accept = -1;


int main(int argc, char *argv[])
{

	char buf[512];
	memset(buf,0,sizeof(buf));
	strncpy(buf, "+++++++ is data +++++++",sizeof(buf));
	cteate_socket_cloud();
	accept_socket_cloud();
	send_data_cloud(buf, strlen(buf));


	return 0;
}

int cteate_socket_cloud()
{
	struct sockaddr_un server_address;
	int addr_len = 0;

	unlink(CLOUD_PATH);
	memset(&server_address, 0, sizeof(struct sockaddr_un));
	sock_server = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock_server < 0)
	{
		LOG("[server]sock_server create filed\n");
		return -1;
	}
	LOG("[server]server socket_cloud is %d\n",sock_server);
	
	server_address.sun_family = AF_UNIX;
	strncpy (server_address.sun_path, CLOUD_PATH ,sizeof(CLOUD_PATH));
	addr_len = offsetof(struct sockaddr_un, sun_path) + strlen(server_address.sun_path);
	
	if(bind(sock_server, (struct sockaddr *)&server_address, addr_len) < 0)
	{
		LOG("[server]bind is filed\n");
		perror("bind()");
		return -1;
	}
	
	listen(sock_server, 5);
	return sock_server;
}

int accept_socket_cloud()
{
	struct sockaddr_un accept_addr;
	int addr_len = sizeof(struct sockaddr_un);
	char *name = NULL;
	name = malloc(sizeof(accept_addr.sun_path)+1);
	memset(&accept_addr, 0, sizeof(struct sockaddr_un));
	
	LOG("[server]accepting .......\n");
	sock_accept = accept(sock_server, (struct sockaddr *)&accept_addr, (socklen_t *)&addr_len);
	if(sock_accept < 0)
	{
		LOG("[server]accept is filed\n");
		return -1;
	}
	addr_len -= offsetof(struct sockaddr_un, sun_path);
	memcpy(name, accept_addr.sun_path,addr_len);
	name[addr_len] = 0;
	LOG("[server]sock_accept id=%d,accept_addr.sun_path=%s\n",sock_accept,name);
	free(name);
	name = NULL;
	return sock_accept;
}

int send_data_cloud(char *buf, int len)
{
	int ret;
	printf("[server]buf = %s,len = %d\n",buf,len);
	if((buf == NULL) || (len <= 0) || (sock_accept < 0))
	{
		LOG("[server]buf is error\n");
		return -1;
	}
	while(8){
		LOG("[server]before send data\n");
		ret = recv(sock_accept, buf, len,0);
		if(ret < 0)
		{
			LOG("[server]send data filed\n");
			perror("send()");
			return -1;
		}
		LOG("[server] write ret=%d\n",ret);
		sleep(1);
	}
	close(sock_server);
	close(sock_accept);

}

