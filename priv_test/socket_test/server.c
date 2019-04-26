#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXBUF 1024

int main(int argc, char **argv) {
	int sockfd, new_fd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport, lisnum;
	char buf[MAXBUF + 1];

	if (argv[1])
		myport = atoi(argv[1]);
	else
		myport = 1989;

	if (argv[2])
		lisnum = atoi(argv[2]);
	else
		lisnum = 5;


	/* 开启一个 socket 监听 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	} else
		printf("socket created\n");

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(myport);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		perror("bind");
		exit(1);
	} else
		printf("binded\n");

	if (listen(sockfd, lisnum) == -1) {
		perror("listen");
		exit(1);
	} else
		printf("begin listen\n");

	while (1) {
		len = sizeof(struct sockaddr);
		/* 等待客户端连上来 */
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len))
				== -1) {
			perror("accept");
			exit(errno);
		} else
			printf("server: got connection from %s, port %d, socket %d\n",
					inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port),
					new_fd);


		/* 开始处理每个新连接上的数据收发 */
		bzero(buf, MAXBUF + 1);
		strcpy(buf, "1111");
		/* 发消息给客户端 */
		close(new_fd);
	}
	return 0;
}




