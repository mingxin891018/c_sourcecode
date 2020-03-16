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

#define MAXBUF 4096
#define MYPORT 32769
#define MYADDR "10.10.3.16"
//#define MYADDR "192.168.1.102"
#define log_debug(format, ...) printf("[E][%s %s %d] "format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__)

int main(int argc, char **argv) {
	int sockfd = -1, new_fd = -1, dfd = -1, ret = -1;
	struct sockaddr_in my_addr, their_addr;
	socklen_t sockaddr_len = 0;
	unsigned int lisnum = 5;
	char buf[MAXBUF + 1] = {0};
	char send_buf[MAXBUF + 1] = {0};
	size_t len= 0, send_len = 0, json_len = 0;
	unsigned char *p = NULL;

	/* 开启一个 socket 监听 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		log_debug("cteate socket failed!\n");
		goto GET_END;
	} else
		log_debug("socket created\n");

	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr=inet_addr(MYADDR);

	int reuse = 1;
	setsockopt(sockfd , SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

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
		while(1)
		{
			//接收客户端支持的加密方式
			//p = "{\"type\":\"keyngack\",\"sequence\": 1,\"mac\":\"00112233ABCD\",\"keymode\":\"ecdh\"}";
			memset(buf, 0, sizeof(buf));
			len = read(new_fd, buf, MAXBUF);
			if(len < 0)
			{
				log_debug("read support keymode failed\n");
				goto again;
			}
			log_debug("read length=%zu, data=%s\n", len, &buf[8]);

			//发送通讯使用的加密方式
			p = "{\"type\":\"keyngack\",\"sequence\": 1,\"mac\":\"00112233ABCD\",\"keymode\":\"dh\"}";
			json_len = strlen(p);
			memset(send_buf, 0, sizeof(send_buf));
			send_buf[0] = 0x3f;
			send_buf[1] = 0x72;
			send_buf[2] = 0x1f;
			send_buf[3] = 0xb5;
			send_buf[4] = (0xff000000  & json_len) >> 24;
			send_buf[5] = (0xff0000    & json_len) >> 16;
			send_buf[6] = (0xff00      & json_len) >> 8;
			send_buf[7] = (0xff        & json_len);
			memcpy(&send_buf[8], p, strlen(p));
			send_len = json_len + 8;
			ret = send(new_fd, send_buf, send_len, 0);
			if(ret != send_len)
			{
				log_debug("send command failed!,ret=%zu\n", len);
				goto again;
			}
			log_debug("send used keymode length=%zu, data=%s\n", send_len, p);

			//接收客户端发送的公钥
			//p = "{\"type\": \"ecdh\",\"sequence\": 2,\"mac\": \"00112233ABCD\",\"data\":{\"ecdh_key\": \"ZGNiYTk4NzY1NDMyMTA=\"}}";
			memset(buf, 0, sizeof(buf));
			len = read(new_fd, buf, MAXBUF);
			if(len < 0)
			{
				log_debug("read data failed\n");
				goto again;
			}
			log_debug("read pubkey length=%zu, data=%s\n", len, &buf[8]);
			char *p = strstr(&buf[8], "dh_key");
			char *q = strstr(p + 1, "\"");
			char *r = strstr(q + 1, "\"");
			char *m = strstr(r + 1, "\"");
			*m = 0;
			int pubkey_len = 0;
			set_pubkey(r+1, strlen(r+1));
			

			//发送服务器端的公钥
			//p = "{\"type\":\"dh\",\"sequence\":1,\"mac\":\"00112233ABCD\",\"data\":{\"dh_key\":\"ttV/OMmOubfQkbXVNycJWw==\",\"dh_p\":\"5padPUlb4yx88YDDvdR5jg==\",\"dh_g\":\"BQ==\"}}";
			char buf_tmp[512] = {0};
			get_send_pubkey(&m, &pubkey_len);
			log_debug("m=%s\n", m);
			snprintf(buf_tmp, sizeof(buf_tmp), "{\"type\":\"dh\",\"sequence\":1,\"mac\":\"00112233ABCD\",\"data\":{\"dh_key\":\"%s\",\"dh_p\":\"0HIlW8scDfdzsgMMATg0Dw==\",\"dh_g\":\"BQ==\"}}", m);
			json_len = strlen(buf_tmp);
			memset(send_buf, 0, sizeof(send_buf));
			send_buf[0] = 0x3f;
			send_buf[1] = 0x72;
			send_buf[2] = 0x1f;
			send_buf[3] = 0xb5;
			send_buf[4] = (0xff000000  & json_len) >> 24;
			send_buf[5] = (0xff0000    & json_len) >> 16;
			send_buf[6] = (0xff00      & json_len) >> 8;
			send_buf[7] = (0xff        & json_len);
			memcpy(&send_buf[8], buf_tmp, strlen(buf_tmp));
			send_len = json_len + 8;
			ret = send(new_fd, send_buf, send_len, 0);
			if(ret != send_len)
			{
				log_debug("send command failed!,ret=%zu\n", len);
				goto again;
			}
			log_debug("send pubkey length=%zu, data=%s\n", send_len, &send_buf[8]);

			while(1)
			{
				static count = 0;
				if(count >= 5)
					break;
				memset(buf, 0, sizeof(buf));
				len = read(new_fd, buf, MAXBUF);
				if(len < 0)
				{
					log_debug("read data failed\n");
					goto again;
				}
				log_debug("recv register package,read length=%zu, data=%s\n", len, &buf[8]);
				analysis_register_data(buf, len);
				sleep(2);

				p = "{\"type\": \"ack\",\"sequence\": 4,\"mac\": \"00112233ABCD\"}";
				json_len = strlen(p);
				memset(send_buf, 0, sizeof(send_buf));
				send_buf[0] = 0x3f;
				send_buf[1] = 0x72;
				send_buf[2] = 0x1f;
				send_buf[3] = 0xb5;
				send_buf[4] = (0xff000000  & json_len) >> 24;
				send_buf[5] = (0xff0000    & json_len) >> 16;
				send_buf[6] = (0xff00      & json_len) >> 8;
				send_buf[7] = (0xff        & json_len);
				memcpy(&send_buf[8], p, strlen(p));
				send_len = json_len + 8;
				ret = send(new_fd, send_buf, send_len, 0);
				if(ret != send_len)
				{
					log_debug("send command failed!,ret=%zu\n", len);
					goto again;
				}
				log_debug("send register ack length=%zu, data=%s\n", send_len, p);
				count++;
				
			}

			p = "{\"type\": \"qlink\",\"sequence\": 58,\"mac\": \"00112233ABCD\",\"result\": \"allow\",\"wifi\": [{\"radio\": {\"mode\": \"2.4G\",\"currentChannel\": 8},\"ap\": [{\"ssid\": \"ChinaNet-3567\",\"key\": \"1234567890\",\"auth\": \"wpapskwpa2psk\",\"encrypt\": \"aes\"}]},{\"radio\": {\"mode\": \"25G\",\"currentChannel\": 149},\"ap\": [{\"ssid\":\"ChinaNet-3567-5G\",\"key\": \"1234567890\",\"auth\": \"wpapskwpa2psk\",\"encrypt\": \"aes\"}]}]}";
			json_len = strlen(p);
			memset(send_buf, 0, sizeof(send_buf));
			send_buf[0] = 0x3f;
			send_buf[1] = 0x72;
			send_buf[2] = 0x1f;
			send_buf[3] = 0xb5;
			send_buf[4] = (0xff000000  & json_len) >> 24;
			send_buf[5] = (0xff0000    & json_len) >> 16;
			send_buf[6] = (0xff00      & json_len) >> 8;
			send_buf[7] = (0xff        & json_len);
			memcpy(&send_buf[8], p, strlen(p));
			send_len = json_len + 8;
			ret = send(new_fd, send_buf, send_len, 0);
			if(ret != send_len)
			{
				log_debug("send command failed!,ret=%zu\n", len);
				goto again;
			}
			log_debug("send register result,length=%zu, data=%s\n", send_len, p);

			memset(buf, 0, sizeof(buf));
			len = read(new_fd, buf, MAXBUF);
			if(len < 0)
			{
				log_debug("read data failed\n");
				goto again;
			}
			log_debug("read overack length=%zu, data=%s\n", len, &buf[8]);
again:
			if(new_fd > 0)
			{
				close(new_fd);
				new_fd = -1;
			}
			break;
		}
	}


GET_END:
	if(sockfd >= 0)
		close(sockfd);
	if(new_fd >= 0){
		close(new_fd);
		new_fd = -1;
	}
	return 0;
}


