#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "demo_priv.h"

int main(int argc, char **argv) {
	int sockfd = -1, new_fd = -1, dfd = -1, ret = -1;
	struct sockaddr_in my_addr, their_addr;
	socklen_t sockaddr_len = 0;
	unsigned int lisnum = 1;
	char buf[MAXBUF + 1] = {0};
	size_t len= 0, pos = 0;

	/* 开启一个 socket 监听 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_debug("cteate socket failed!\n");
		goto GET_END;
	} else
		log_debug("socket created\n");

	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr=inet_addr(MYADDR);
	
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
		log_debug("bind error!\n");
		goto GET_END;
	} else
		log_debug("bind success!\n");

	if (listen(sockfd, lisnum) == -1) {
		log_debug("listen error!\n");
		goto GET_END;
	} else
		log_debug("begin listen,ip=%s,port=%d\n", MYADDR, MYPORT);

	while (1) {
		sockaddr_len = sizeof(struct sockaddr);
		/* 等待客户端连上来 */
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sockaddr_len))== -1) {
			log_debug("accept failed!,ret=%d\n", new_fd);
			goto GET_END;
		} else
			log_debug("server: got connection from %s, port %d, socket %d\n",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port),new_fd);

		/* 开始处理每个新连接上的数据收发 */
		memset(buf, 0, sizeof(buf));
		strcpy(buf, COMMAND_JGP);

		if(len = send(new_fd, buf, strlen(buf), 0) <= 0){
			log_debug("send command failed!,ret=%zu\n", len);
			goto GET_END;
		}
	
		if((dfd = open(JPG_RECV_PATH,O_WRONLY|O_TRUNC|O_CREAT, 0644)) < 0){
			log_debug("open jpg file failed!\n");
			goto GET_END;
		}
		log_debug("create file[%s] success!\n", JPG_RECV_PATH);

		while(1)
		{
			memset(buf, 0, sizeof(buf));
			len = read(new_fd, buf, MAXBUF);
			if(len < 0)
			{
				log_debug("read jgp data failed\n");
				goto GET_END;
			}
			log_debug("read[%s] length=%zu, data=%s\n", JPG_RECV_PATH, len, buf);
			if(len == 0){
				log_debug("recv data over!\n");
				break;
			}
			
			pos = 0;
			while(len > 0)
			{
				ret = write(dfd,buf+pos,len);
				if(ret < 0){
					log_debug("write jgp data failed!\n");
					goto GET_END;
				}
				len -= ret;
				pos += ret;
			}

		}
		if(new_fd >= 0){
			close(new_fd);
			new_fd = -1;
		}
		if(dfd >= 0){
			close(dfd);
			dfd = -1;
		}
	}

GET_END:
	if(sockfd >= 0)
		close(sockfd);
	if(new_fd >= 0){
		close(new_fd);
		new_fd = -1;
	}
	if(dfd >= 0){
		close(dfd);
		dfd = -1;
	}
	return 0;
}




