#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "demo_priv.h"

int get_jpg_file(void)
{
	return 0;
}

int main(int argc, char **argv)
{
	int sockfd = -1, file_fd = -1, ret = -1;
	size_t len = -1, pos = 0;
	struct sockaddr_in dest;
	char buffer[MAXBUF + 1] = {0};

	/* 创建一个 socket 用于 tcp 通信 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_debug("cteate socket failed!\n");
	}
	log_debug("socket created\n");

	/* 初始化服务器端（对方）的地址和端口信息 */
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(MYPORT);
	dest.sin_addr.s_addr=inet_addr(MYADDR);

	/* 连接服务器 */
	if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
		log_debug("connect server failed!\n");
		goto finish;
	}
	log_debug("server connected\n");

	/* 接收对方发过来的消息，最多接收 MAXBUF 个字节 */
	memset(buffer, 0, sizeof(buffer));

	if((len = recv(sockfd, (void*)buffer, sizeof(buffer), 0)) <= 0){
		log_debug("recv command failed!,ret=%zu\n", len);
		goto finish;
	}

	if(strcmp(buffer, COMMAND_JGP) != 0){
		log_debug("get command[%s] failed!\n", COMMAND_JGP);
		goto finish;
	}
	log_debug("get command success!\n");

	get_jpg_file();
	file_fd = open(JPG_PATH, O_RDONLY, 0644);
	if(file_fd < 0){
		log_debug("open jpg file failed!\n");
		goto finish;
	}

	log_debug("start read file,fd=%d!\n", file_fd);
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		if((len = read(file_fd, buffer, MAXBUF)) < 0)
		{
			log_debug("read jgp data failed\n");
			goto finish;
		}
		log_debug("read %s data=%s,len=%zu\n", JPG_PATH, buffer, len);

		if(len == 0){
			log_debug("read over!\n");
			break;
		}

		pos = 0;
		while(len > 0)
		{
			ret = write(sockfd,buffer+pos,len);
			if(ret < 0){
				log_debug("write jgp data failed!\n");
				goto finish;
			}
			len -= ret;
			pos += ret;
		}
	}

finish:
	/* 关闭连接 */
	if(sockfd > 0)
		close(sockfd);
	if(file_fd > 0)
		close(file_fd);
	return 0;
}
