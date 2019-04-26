#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  

#define CLOUD_PATH "/tmp/cloud.tmp"

static int client_sockfd = 0;
int create_client_sock();
int connect_server(const char *path_name);
int read_data(char *buf,int len);
	

int main (int argc, char *argv[])  
{
	char buf[512] = {0};
	memset(buf, 0 ,sizeof(buf));
	int fd = create_client_sock();
	if (fd < 0)
		return 0;
	printf("fd = %d\n",fd);
	int accfd = connect_server(CLOUD_PATH);
	printf("accfd = %d\n",accfd);
	while(1){
	read_data(buf,sizeof(buf));
	printf("data = %s\n",buf);
	}
}

int create_client_sock()
{
	if ((client_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror ("socket");
		return -1;
	}

	return client_sockfd;

}

int connect_server(const char *path_name)
{
	int len = 0, result = 0;
	struct sockaddr_un address;
	
	address.sun_family = AF_UNIX;
	strcpy (address.sun_path, path_name);
	len = sizeof (address);
	
	result = connect (client_sockfd, (struct sockaddr *)&address, len);
	if (result < 0) {   
		perror ("connect");  
		return -1;
	}  
	return result;
}


int read_data(char *buf,int len)
{
	int bytes = 0, ret = 0;
	fd_set fds;
	struct timeval timeout={3,0};
	
	FD_ZERO(&fds);
	FD_SET(client_sockfd,&fds);
	ret = select(client_sockfd+1,&fds,NULL,NULL,&timeout);
	if(ret <= 0)
		return 0;
	else
	{
		if ((bytes = read (client_sockfd, buf, len)) == -1) { /*接收消息*/
			perror ("read");  
			exit (EXIT_FAILURE);  
		}   
	}
	return bytes;
}




