/***********************************************************************************************
* CONTENT: 与Http服务器建立连接
* PARAM:
	[in] ipv6: http服务器地址
	[in] port: http服务器端口
* RETURN:
	还回连接状态
* NOTE:
************************************************************************************************/
HANDLE sw_httpclient_connect_v6( struct in6_addr ipv6, unsigned short port, int timeout )
{
	http_client_t* p_http_client = NULL; 
	unsigned long unblock = 1;
	fd_set wset, rset;
	int n;
	int retcode;
	unsigned int now;
	static bool first = true;	
	struct linger lingerOpt;	
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;
    char addr[64];
	memset(addr, 0, sizeof(addr));
	inet_ntop(AF_INET6,&ipv6, addr, sizeof(addr) );
	p_http_client = httpclient_malloc( sizeof(http_client_t));	
	SWCOMMON_LOG_DEBUG("%p, [%s]:%d, timeout=%d, ref=%d\n",p_http_client, addr, ntohs(port), timeout, m_ref);

	if( p_http_client == NULL )
		goto ERROR_EXIT;

	p_http_client->error_code = HTTP_OK;
	strlcpy(p_http_client->version, "1.1", sizeof(p_http_client->version));
	p_http_client->ipv6 = ipv6;
	p_http_client->port = port;
	
	if( m_sz_cookies[0] != '\0' )//m_sz_cookies需要确保有结束符并且缓冲大小一致
	  	strlcpy(p_http_client->sz_cookies, m_sz_cookies, sizeof(p_http_client->sz_cookies));
	else
	  	memset( p_http_client->sz_cookies, 0, sizeof(p_http_client->sz_cookies) );

	/* 创建socket */
	p_http_client->skt = sw_tcp_socket_v6();
	if( p_http_client->skt < 0 )
		goto ERROR_EXIT;
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( p_http_client->skt, FIONBIO, &unblock ) < 0 )
		goto ERROR_EXIT;
	{
		int reuse = 1;
		setsockopt(p_http_client->skt , SOL_SOCKET, SO_REUSEADDR, 
				   (char *)&reuse, sizeof(reuse));	
		//setsockopt( p_http_client->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );
	}

	now = sw_thrd_get_tick();

	if( first )
	{
		srand( now );
		sw_tcp_bind_v6(p_http_client->skt, in6addr_any, htons( (unsigned short) (30000 + rand()%26000 )));
		first = false;
	}
	/* 连接... */
	errno = 0;
	retcode = sw_tcp_connect_v6( p_http_client->skt, ipv6, port );
	if(retcode == 0)
		return p_http_client;
	if(errno != 0 && errno != EINPROGRESS)
	{
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
		SWCOMMON_LOG_ERROR("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
		goto ERROR_EXIT;
	}
	
	n=0;
WAIT:
	/* 等待连接成功 */
	if( (retcode = sw_tcp_select( p_http_client->skt, &rset, &wset, NULL, timeout )) < 0 )
		goto ERROR_EXIT;
	if(retcode == 0)
	{
		SWCOMMON_LOG_ERROR("[HTTPCLIENT]select socket timeout! [%s]:%d\n", addr, ntohs(port));
		goto ERROR_EXIT;
	}

	if( FD_ISSET( p_http_client->skt, &rset ) )
	{
		char sz_buf[16];
		int readsize = sw_tcp_recv( p_http_client->skt, sz_buf, sizeof(sz_buf) );
		if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
		{
			if( ++n<3 )
				goto WAIT;
		}
	}
	else if( FD_ISSET( p_http_client->skt, &wset ) )
	{
		int err;
		int len = sizeof(err);
#ifndef WIN32
		if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) 
#else
		if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0) 
#endif
		{
			SWCOMMON_LOG_ERROR("[HTTPCLIENT]getsocketopt failed!\n");
			goto ERROR_EXIT;
		}

		if (err) {
			printf ("[HTTPCLIENT] connect failed erron:%d!\n", err);
			goto ERROR_EXIT;
		}
		return p_http_client ;
	}
	
	if(n==2)
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
	else
	{
		if( errno==ECONNREFUSED )
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
		else
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
	}
	
ERROR_EXIT:
	httpclient_free(p_http_client);
	return NULL;
}
