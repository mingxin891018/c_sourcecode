#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <sys/un.h>  
#include <unistd.h>  
#include <stdlib.h>  

#define CLOUD_PATH "/tmp/cloud.tmp"

static int server_sockfd = -1;
static int accept_sockfd = -1;
static struct sockaddr_un server_address;
static struct sockaddr_un accept_address;


int create_cloud_sock(const char *path_name);
int wait_accept_sock();
int send_data(const char *buf,int len);



int main (int argc, char *argv[])  
{
	int sockfd = -1;
	int acceptfd = -1;
	int count = 0;
	sockfd = create_cloud_sock(CLOUD_PATH);
	printf("sockfd = %d\n",sockfd);

	acceptfd = wait_accept_sock();
	printf("acceptfd = %d\n",acceptfd);
	while(1)	
	{
		send_data("123456",6);
		printf("count=%d\n",count++);
		sleep(3);
	}
}

int create_cloud_sock(const char *path_name)
{
	int server_len = 0;
	unlink (path_name); /*删除原有server_socket对象*/
	server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0);
	if(server_sockfd < 0)
	{
		perror("socket");
		return -1;
	}
	server_address.sun_family = AF_UNIX;
	strcpy (server_address.sun_path, path_name);
	server_len = sizeof (server_address);

	if(bind (server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
	{
		perror("bend");
		return -1;
	}
	listen (server_sockfd, 5);
	return server_sockfd;
}

int wait_accept_sock()
{
	int client_len = sizeof(accept_address);
	accept_sockfd = accept (server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);
	if(accept_sockfd < 0)
	{
		perror("accept");
		return -1;
	}
	return accept_sockfd;
}

int send_data(const char *buf,int len)
{
	int bytes = 0;
	if ((bytes = write (accept_sockfd, buf, len)) == -1) {
		perror("write");
		return -1;
	}
	return bytes;
}



