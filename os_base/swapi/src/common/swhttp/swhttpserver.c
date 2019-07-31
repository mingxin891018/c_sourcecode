#include "swapi.h"
#include "swhttpserver.h"
#include "swtcp.h"
#include "swmem.h"
#include "swthrd.h"
#include "swlog.h"
#include "base64.h"
#include "swbase.h"
#include "swcommon_priv.h"

typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 服务器ip，网络字节序 */
	unsigned long ip;
	/* 服务器端口 */
	unsigned short port;
	/*服务器线程*/
	HANDLE  thrd;
	/*回调函数*/
	http_server_callback httpserver_callback;
	/*回调函数的参数*/
	unsigned long callback_param;
}http_server_t;

typedef struct http_enum_string
{
	http_response_num_t type;
	const char *name;
	const char *info;
} http_enum_string_t;

static const http_enum_string_t http_resp_name[] = {
	{ HTTP_OK, "OK", "" },
	{ HTTP_MOVED_TEMPORARILY, "Found", "Directories must end with a slash." },
	{ HTTP_REQUEST_TIMEOUT, "Request Timeout",
	  "No request appeared within a reasonable time period." },
	{ HTTP_NOT_IMPLEMENTED, "Not Implemented",
	  "The requested method is not recognized by this server." },
	{ HTTP_UNAUTHORIZED, "Unauthorized", "" },
	{ HTTP_NOT_FOUND, "Not Found",
	  "The requested URL was not found on this server." },
	{ HTTP_BAD_REQUEST, "Bad Request", "Unsupported method." },
	{ HTTP_FORBIDDEN, "Forbidden", "" },
	{ HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error",
	  "Internal Server Error" },
	{ HTTP_CREATED, "Created","Created" },
	{ HTTP_ACCEPTED, "Accepted","Accepted"},
	{ HTTP_NO_CONTENT, "No Content","No Content"},
	{ HTTP_MULTIPLE_CHOICES, "Multiple Choices","Multiple Choices"},
	{ HTTP_MOVED_PERMANENTLY, "Moved Permanently","Move Permanently"},
	{ HTTP_NOT_MODIFIED, "Not Modified","Not Modified"},
	{ HTTP_BAD_GATEWAY, "Bad Gateway", "" },
	{ HTTP_SERVICE_UNAVAILABLE, "Service Unavailable", "" },
};
static const char RFC1123FMT[] = "%a, %d %b %Y %H:%M:%S GMT";
static bool http_server_proc( uint32_t wparam, uint32_t lparam );

/** 
 * 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 返回Httpserver句柄
 */
HANDLE sw_httpserver_open(unsigned short port, http_server_callback callback, uint32_t wparam )
{
	int fd;
	int on = 1;
	http_server_t* p_server = NULL;
	
	fd = sw_tcp_socket();
	if(fd < 0)
	{
		SWCOMMON_LOG_FATAL("sw_httpserver_open::Create socket failed!\n");
		return NULL;
	}

#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (void *)&on, sizeof(on)) ;
#else
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)) ;
#endif
	sw_tcp_bind(fd, INADDR_ANY, port);
	sw_tcp_listen(fd, 5);
	if((p_server = malloc(sizeof(http_server_t))) == NULL)
	{
		SWCOMMON_LOG_FATAL("[HTTPSERVER]malloc failed!\n");
		return NULL;
				
	}

	memset(p_server, 0, sizeof(http_server_t));
	p_server->skt = fd;
	p_server->ip = INADDR_ANY;
	p_server->port = port;
	
	p_server->httpserver_callback = callback;
	p_server->callback_param = wparam;
	p_server->thrd = sw_thrd_open( "tHttpServer", 80, 0, 16384, http_server_proc, (unsigned long)p_server, 0 );

	if(p_server->thrd == NULL)
	{
		SWCOMMON_LOG_FATAL("[HTTPSERVER]sw_thrd_open failed!\n");
		free(p_server);
		return NULL;	
	}

	sw_thrd_resume( p_server->thrd );
	return p_server;
}


/** 
 * 关闭httpserver
 * 
 * @param server 
 */
void sw_httpserver_close( HANDLE server )
{
	http_server_t* p_server = (http_server_t*)server;
	
	if( p_server->skt != -1)
			shutdown(p_server->skt,2);

	if(p_server->thrd)
	{
		sw_thrd_close(p_server->thrd, 5000);
		p_server->thrd = NULL;
	}
	p_server->httpserver_callback = NULL;
	p_server->callback_param = 0;
	
	sw_tcp_close(p_server->skt);
	p_server->skt = -1;
	free(p_server);	
}
	
static int get_line(http_connect_obj_t* obj, int timeout)
{
	int  count = 0;
	char *buf = obj->buf;
	fd_set rset;

	if( sw_tcp_select( obj->skt, &rset, NULL, NULL, timeout ) < 0 )
	{
		printf("time out\n");
		return -1;
	}
	if( FD_ISSET(obj->skt, &rset) )
	{
		while (sw_tcp_recv(obj->skt, buf + count, 1) == 1)
		{
			obj->request_header.header_length++;
		  
			if (buf[count] == '\r') continue;
			if (buf[count] == '\n') {
				buf[count] = 0;
				return count;
			}
			if(count < (HTTPSERVER_MAX_LINE - 1))      /* check owerflow */
				count++;
		}
	}
	if (count) return count;
	else return -1;
}

/** 
 * 接收Request报头， 成功则填充request_header结构
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: 成功返回报文头的大小， 失败返回负数
 */
int sw_httpserver_recv_request_header(http_connect_obj_t* obj, int timeout )
{
	char* p = NULL, *last = NULL;
	
	memset(&obj->request_header, 0, sizeof( http_request_header_t ));
	
	do
	{
		if(get_line(obj,timeout) <= 0)
			break;
		if(strncasecmp(obj->buf, "GET", strlen("GET")) == 0 || strncasecmp(obj->buf, "POST", strlen("POST")) == 0 )
		{
			last = NULL;
			p = strtok_r(obj->buf, " ", &last);
			if(p == NULL)
				continue;
			strlcpy(obj->request_header.method, p, sizeof(obj->request_header.method));
			p = strtok_r(obj->buf, " ", &last);
			if(p == NULL)
				continue;

			strlcpy(obj->request_header.request_url, p, sizeof(obj->request_header.request_url));
			
		}
		else if(strncasecmp(obj->buf, "Host:", strlen("Host:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.host, p+1, sizeof(obj->request_header.host));

		}
		else if(strncasecmp(obj->buf, "Accept:", strlen("Accept:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.accept_type, p+1, sizeof(obj->request_header.accept_type));
			
		}
		else if(strncasecmp(obj->buf, "Accept-Encoding:", strlen("Accept-Encoding:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.accept_encoding, p+1, sizeof(obj->request_header.accept_encoding));
			
		}	
		else if(strncasecmp(obj->buf, "Content-Type:", strlen("Content-Type:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.content_type, p+1, sizeof(obj->request_header.content_type));
			
		}
		else if(strncasecmp(obj->buf, "Content-Length:", strlen("Content-Length:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			obj->request_header.content_length = atoi(p+1);			
		}	
		else if(strncasecmp(obj->buf, "Connection:", strlen("Connection:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.connection, p+1,  sizeof(obj->request_header.connection));
			
		}			
#ifdef HTTP_AUTHENTICATION_REQUIRED
		else if(strncasecmp(obj->buf, "Authorization:", strlen("Authorization:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.authorization, p+1,  sizeof(obj->request_header.authorization));
		}
#endif
	}while(1);

	SWCOMMON_LOG_DEBUG("[HTTPSERVER GET REQUEST]\n");
	SWCOMMON_LOG_DEBUG("[METHOD]:%s\n", obj->request_header.method);
	SWCOMMON_LOG_DEBUG("[URL]:%s\n", obj->request_header.request_url);
	SWCOMMON_LOG_DEBUG("[HOST]:%s\n", obj->request_header.host);
	SWCOMMON_LOG_DEBUG("[ACCEPT_TYPE]:%s\n", obj->request_header.accept_type);
	SWCOMMON_LOG_DEBUG("[CONNECTION]:%s\n", obj->request_header.connection);
	SWCOMMON_LOG_DEBUG("[CONTENT_TYPE]:%s\n", obj->request_header.content_type);
	SWCOMMON_LOG_DEBUG("[CONTENT_LENGHT]:%d\n", obj->request_header.content_length);

	return obj->request_header.header_length;	
}

/** 
 * 接收请求实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回接收数据的字节数， 0: 接收完成， -1: 接收出错
 */
int sw_httpserver_recv_request_content(http_connect_obj_t* obj, char*buf, int buf_size, int timeout )
{
	if(obj->request_header.content_length > 0)
	{
		fd_set rset;
		if( sw_tcp_select( obj->skt, &rset, NULL, NULL, timeout ) < 0 )
		{
			printf("time out\n");
			return 0;
		}
		if( FD_ISSET(obj->skt, &rset) )
			return sw_tcp_recv(obj->skt, buf, buf_size);
	}
	return 0;
}

/** 
 * 
 * 发送响应报文头
 * @param obj 
 * @param responseNum 
 * @param content_type 
 * @param content_encoding 
 * @param connection 
 * @param content_length 
 * @param timeout 
 * 
 * @return 发送的数据字节数
 */
int sw_httpserver_send_response_header(http_connect_obj_t* obj, http_response_num_t responseNum, 
									   char* content_type, char* content_encoding, 
									   char* connection,
									   int content_length, 
									   int timeout)
{
	if (obj == NULL)
		return -1;
	char *buf = obj->buf;
	int size = sizeof(obj->buf);
	const char *resp_string = "";
	const char *info_string = NULL;
	time_t timer = time(0);
	char time_str[80];
	int i, len;

	for (i = 0; i<(int)(sizeof(http_resp_name)/sizeof(http_resp_name[0])); i++) 
	{
		if (http_resp_name[i].type == responseNum) 
		{
			resp_string = http_resp_name[i].name;
			info_string = http_resp_name[i].info;
			break;
		}
	}

	if(info_string == NULL)
	{
		SWCOMMON_LOG_FATAL("Wrong response number[%d]\n",  responseNum);
		return -1;
		
	}

	/* emit the current date */
	strftime(time_str, sizeof(time_str), RFC1123FMT, gmtime(&timer));

	len = snprintf(buf, size, 
				  "HTTP/1.1 %d %s\r\n"
				  "Date: %s\r\n",
				  responseNum, resp_string, time_str);
#ifdef HTTP_AUTHENTICATION_REQUIRED
	if(responseNum == HTTP_UNAUTHORIZED)
	{
		srand( (unsigned)time( NULL ) + rand()*2 );
		len += snprintf(buf+len, len < size ? size-len : 0, "WWW-Authenticate: Digest realm=\"HttpDigestAuthentication\", qop=\"auth\", nonce=\"%d\", opaque=\"abcd01082883008ab01082883008abcd\"\r\n", rand());
	}
#endif
	if(content_type)
		len += snprintf(buf+len, len < size ? size-len : 0, "Content-type: %s\r\n", content_type);
	if(content_encoding)
		len += snprintf(buf+len, len < size ? size-len : 0, "Content-Encoding: %s\r\n", content_encoding);
	if(content_length)
		len += snprintf(buf+len, len < size ? size-len : 0, "Content-Length: %d\r\n", content_length);
	if(connection)
		len += snprintf(buf+len, len < size ? size-len : 0, "Connection: %s\r\n", connection);
    
	len += snprintf(buf+len, len < size ? size-len : 0, "\r\n");

	SWCOMMON_LOG_DEBUG("[HTTPSERVER RESPONSE]%d\n%s\n", len, buf);
	if (len > size)
		return -1;

	i = 0;
	while(i < len)
	{
		int ret = 0;
		ret = sw_tcp_send(obj->skt, buf + i, len - i);
		if( ret <= 0 )
		{
			SWCOMMON_LOG_ERROR("sw_tcp_send return error : %d\n", ret);
			/*error has happended , report fail*/
			return 0;
		}
		i += ret;
	}

	return len;
}

/** 
 * 发送响应实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回发送数据字节数 -1：发送失败
 */
int sw_httpserver_send_response_content(http_connect_obj_t* obj,
										char*buf, int buf_size,
										int timeout)
{
	int ret = 0;
	ret = sw_tcp_send(obj->skt, buf, buf_size);
	return ret <= 0 ? 0 : ret;

}

/** 
 * 关闭连接,同时释放资源
 * 
 * @param obj 
 */
void sw_httpserver_close_connectobj(http_connect_obj_t* obj)
{
	if(obj)
	{
		sw_tcp_close(obj->skt);
		free(obj);	
	}
}
	

static bool http_server_proc( uint32_t wparam, uint32_t lparam )
{
	unsigned long accept_ip;
	unsigned short port;
	int skt = -1;
	http_server_t *p_server = (http_server_t *)wparam;
	//if(sw_tcp_select(p_server->skt, &rset, NULL, NULL, 2000) > 0)
	{
		//if(FD_ISSET(p_server->skt, &rset))
		{
			printf("http_server_proc accept\n");
			if((skt = sw_tcp_accept(p_server->skt, &accept_ip, &port)) >= 0)
			{
				http_connect_obj_t* obj = malloc(sizeof(http_connect_obj_t));
				int on = 1;
				
				if(obj == NULL)
				{
					SWCOMMON_LOG_FATAL("[HTTPSERVER] malloc failed!\n");
					return true;
										
				}
				obj->skt = skt;
				obj->from_ip = accept_ip;
				obj->from_port = port;
				obj->h_http_server = p_server;

				setsockopt(skt, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof (on));
				if(p_server->httpserver_callback)
					p_server->httpserver_callback(obj, p_server->callback_param);
				return true;
			}
		}
	}
	p_server->thrd = NULL;
	return false;
}

