/** 
 * @file swhttpclient.h
 * @brief HTTP函数接口
 * @author NiuJiuRu
 * @date 2007-10-29
 */
#include "swapi.h"
#include "swtcp.h"
#include "swurl.h"
#include "swlog.h"
#include "swmutex.h"
#include "swthrd.h"
#include "swhttpauth.h"
#include "base64.h"
#include "swcommon_priv.h"
#include "swhttp_priv.h"

#define HTTPS_CERT "/usr/lib/https.pem"
#define SERVER_CERT "/var/server.pem"
#define RE_CERT "/var/re.pem"
#define TMS_CER	"/usr/local/etc/setting/X509/tms_server.crt"
#ifdef SUPPORT_MKDEVCERT_ONLINE
#define STBDEV_CER "/var/x509/devcert.pem"
#define STBDEV_KEY "/tmp/devcertkey.pem"
#endif

static http_client_t m_all[MAX_HTTPCLIENT_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;
static bool bSetTrustCert = false;
//客户端支持的安全密码套
static const char *m_cipher_list = "ECDHE-RSA-AES256-GCM-SHA384,ECDHE-ECDSA-AES256-GCM-SHA384,ECDHE-RSA-AES256-SHA384,ECDHE-ECDSA-AES256-SHA384,ECDHE-RSA-AES256-SHA,ECDHE-ECDSA-AES256-SHA,DHE-DSS-AES256-GCM-SHA384,DHE-RSA-AES256-GCM-SHA384,DHE-RSA-AES256-SHA256,DHE-DSS-AES256-SHA256,DHE-RSA-AES256-SHA,DHE-DSS-AES256-SHA,ECDHE-RSA-AES128-GCM-SHA256,ECDHE-ECDSA-AES128-GCM-SHA256,ECDHE-RSA-AES128-SHA256,ECDHE-ECDSA-AES128-SHA256,ECDHE-RSA-AES128-SHA,ECDHE-ECDSA-AES128-SHA,DHE-DSS-AES128-GCM-SHA256,DHE-RSA-AES128-GCM-SHA256,DHE-RSA-AES128-SHA256,DHE-DSS-AES128-SHA256,DHE-RSA-AES128-SHA,DHE-DSS-AES128-SHA";

//SSL层错误信息回调函数,实现错误码规格
static int ssl_error_print(const char *info,size_t size,void *c)
{
	SWCOMMON_LOG_ERROR("%s\n",info);
	if(strstr(info,"server_certificate:certificate verify failed"))
		SWCOMMON_LOG_ERROR("verify server certificate failed ,errorcode:136021\n");
	else if(strstr(info,"alert certificate"))
		SWCOMMON_LOG_ERROR("server verify client certificate failed ,errorcode:136022\n");
	return 0;
}
/* 全局Cookies */
static char m_szCookies[1024] = {0,};
/***********************************************************************************************
* CONTENT: 与Http服务器建立连接 带强制认证服务器证书
* PARAM:
	[in] ip: http服务器地址
	[in] port: http服务器端口
	[in] key: 双向认证需要的用户证书私钥
* RETURN:
	还回连接状态
* NOTE:
************************************************************************************************/
HANDLE sw_httpsclient_connect_verify( unsigned long  ip, unsigned short port, int timeout, void *key )
{
	http_client_t* pHttpclient = NULL;
	unsigned long unblock = 1;
	fd_set wset, rset;
	int i, n;
	int retcode;
	unsigned int now;
	static bool first = true;
	struct linger lingerOpt;
	lingerOpt.l_onoff = 1;
    lingerOpt.l_linger = 0;
	int reuse = 1;

	if(	m_mutex == NULL)
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( m_ref < 0 )
	{
		m_ref = 0;

		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
			m_all[i].skt = -1;
	}
	for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			pHttpclient = m_all + i;
			break;
		}
	}
	m_ref++;

	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	SWCOMMON_LOG_DEBUG("[HTTPCLIENT]0x%x, ip=%d, port=%d, timeout=%d, ref=%d\n",pHttpclient, ip, port, timeout, m_ref);

	if( pHttpclient == NULL )
		goto ERROR_EXIT;

	pHttpclient->isHttps = true;
	strcpy(pHttpclient->version, "1.1");
	pHttpclient->ip = ip;
	pHttpclient->port = port;

	if(strlen(m_szCookies) > 0)
	  strlcpy(pHttpclient->sz_cookies, m_szCookies, sizeof(pHttpclient->sz_cookies));
	else
	  memset( pHttpclient->sz_cookies, 0, sizeof(pHttpclient->sz_cookies) );

	SSL_library_init();
	SSL_load_error_strings();

	/*Create a SSL_CTX by SSLv23_client_method(), (SSL Content Text)*/
#ifndef SUPPORT_MKDEVCERT_ONLINE
	pHttpclient->ctx = SSL_CTX_new(SSLv23_client_method());
#else
	pHttpclient->ctx = SSL_CTX_new(TLSv1_2_client_method()); //CV5要求完全符合TLSv1.2
#endif
	if (pHttpclient->ctx == NULL)
	{
		SWCOMMON_LOG_FATAL("SSL_CTX_new failed SSLv23_client_method!!!\n");
		goto ERROR_EXIT;
	}
#ifdef SUPPORT_OPENSSL
	SWCOMMON_LOG_INFO("+++++++++ssl_ctx_new ok client+++++++++\n");
	SSL_CTX_set_cipher_list( pHttpclient->ctx, m_cipher_list );
#endif
	/* 设置证书验证方式,
	 * SSL_VERIFY_NONE:完全忽略验证证书的结果
	 * SSL_VERIFY_PEER:希望验证对方的证书 */
	SSL_CTX_set_verify(pHttpclient->ctx, SSL_VERIFY_PEER, NULL);


	SSL_CTX_load_verify_locations(pHttpclient->ctx,TMS_CER, NULL);

#ifdef SUPPORT_MKDEVCERT_ONLINE
	//设置盒子自己的证书链 CV5双向认证
	if(1!=SSL_CTX_use_certificate_chain_file(pHttpclient->ctx,STBDEV_CER))
	{
		SWCOMMON_LOG_ERROR("fail to set certchain %s\n",STBDEV_CER);
		goto ERROR_EXIT;
	}
	if(key == NULL)
	{
		SWCOMMON_LOG_ERROR("the key is NULL\n");
		goto ERROR_EXIT;
	}
	//设置证书私钥
	if(0 == SSL_CTX_use_PrivateKey(pHttpclient->ctx,(EVP_PKEY *)key))
	{
		SWCOMMON_LOG_ERROR("fail to set %s\n",STBDEV_KEY);
		goto ERROR_EXIT;
	}

	if(0 == SSL_CTX_check_private_key(pHttpclient->ctx))
	{
		SWCOMMON_LOG_ERROR("%s and %s is not match\n",STBDEV_CER,STBDEV_KEY);
		goto ERROR_EXIT;
	}
#endif
	/* 创建socket */
	pHttpclient->skt = sw_tcp_socket();
	if( pHttpclient->skt < 0 )
    {
        SWCOMMON_LOG_DEBUG("+++++++++++++++socket error!++++++++++++\n");
		goto ERROR_EXIT;
    }
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( pHttpclient->skt, FIONBIO, &unblock ) < 0 )
    {
        SWCOMMON_LOG_DEBUG("+++++++++++++++ioctl error!++++++++++++\n");
		goto ERROR_EXIT;
    }
	setsockopt(pHttpclient->skt , SOL_SOCKET, SO_REUSEADDR,
				   (char *)&reuse, sizeof(reuse));
	setsockopt( pHttpclient->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );

	now = sw_thrd_get_tick();
	if( first )
	{
		srand( now );
		sw_tcp_bind(pHttpclient->skt, INADDR_ANY, htons( (unsigned short) (30000 + rand()%26000 )));
		first = false;
	}
	/* 连接... */
	errno = 0;
	retcode = sw_tcp_connect( pHttpclient->skt, ip, port );
	if(retcode == 0)
		return pHttpclient;
	if(errno != 0 && errno != EINPROGRESS)
	{
		SWCOMMON_LOG_ERROR ("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
		goto ERROR_EXIT;
	}
	n=0;
WAIT:
	/* 等待连接成功 */
	if( (retcode = sw_tcp_select( pHttpclient->skt, &rset, &wset, NULL, timeout)) < 0 )
    {
        SWCOMMON_LOG_DEBUG("+++++++++++++++select error!++++++++++++\n");
		goto ERROR_EXIT;
    }

	if(retcode == 0)
	{
		SWCOMMON_LOG_ERROR("[HTTPCLIENT]select socket timeout!\n");
		goto ERROR_EXIT;
	}

	if( FD_ISSET( pHttpclient->skt, &rset ) )
	{
		char szBuf[16];
		int readsize = sw_tcp_recv( pHttpclient->skt, szBuf, sizeof(szBuf) );
		if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
		{
			if( ++n<3 )
				goto WAIT;
		}
	}
	else if( FD_ISSET( pHttpclient->skt, &wset ) )
	{
		int err;
		int len = sizeof(err);
#ifndef WIN32
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0)
#else
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0)
#endif
		{
			SWCOMMON_LOG_ERROR ("[HTTPCLIENT]getsocketopt failed!\n");
			goto ERROR_EXIT;
		}

		if (err)
		{
			printf ("[HTTPCLIENT] connect failed erron:%d!\n", err);
			goto ERROR_EXIT;
		}

		unblock = 0;
		sw_tcp_ioctl( pHttpclient->skt, FIONBIO, &unblock );

		pHttpclient->ssl = SSL_new(pHttpclient->ctx);
		if(pHttpclient->ssl == NULL )
		{
			SWCOMMON_LOG_FATAL("httpsclient SSL_new failed !!!\n");
#ifndef SUPPORT_WOLFSSL
			ERR_print_errors_cb(ssl_error_print,NULL);
#endif
			goto ERROR_EXIT;
		}

		SSL_set_fd(pHttpclient->ssl, pHttpclient->skt);
		/*  1  is fine
		 *  0  is "not successful but was shut down controlled"
		 *	<0 is "handshake was not successful, because a fatal error occurred*/
		if(SSL_connect(pHttpclient->ssl) == -1)
		{
			SWCOMMON_LOG_FATAL("httpsclient SSL_connect failed !!!\n");
#ifndef SUPPORT_WOLFSSL
			ERR_print_errors_cb(ssl_error_print,NULL);
#endif
			goto ERROR_EXIT;
		}
		SWCOMMON_LOG_DEBUG("SSL connection using %s\n",SSL_get_cipher(pHttpclient->ssl));
		if (SSL_get_verify_result(pHttpclient->ssl) != X509_V_OK)
		{/* 可能永远不会进入这里，要退出在ssl_connect上就退出了*/
			SWCOMMON_LOG_DEBUG("SSL certificate verify failed\n");
#ifndef SUPPORT_WOLFSSL
			ERR_print_errors_cb(ssl_error_print,NULL);
#endif
			goto ERROR_EXIT;
		}
#ifdef SUPPORT_MKDEVCERT_ONLINE
		//TODO 如果终端校验成功,最后还不成功,可认为是服务器校验不过导致的
		if(SSL_get_state(pHttpclient->ssl) != SSL_ST_OK)
		{
			SWCOMMON_LOG_ERROR("SSL last state is not OK\n");
#ifndef SUPPORT_WOLFSSL
			ERR_print_errors_cb(ssl_error_print,NULL);
#endif
			goto ERROR_EXIT;
		}
#endif
		SWCOMMON_LOG_DEBUG("SSL certificate verify result OK\n");

		return pHttpclient;
	}

ERROR_EXIT:
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if(pHttpclient )
	{
		if( pHttpclient->ssl )
		{
			SSL_shutdown(pHttpclient->ssl);
			SSL_free( pHttpclient->ssl);
			pHttpclient->ssl = NULL;
		}

		if( 0 <= pHttpclient->skt )
		{
			sw_tcp_close( pHttpclient->skt );
			pHttpclient->skt = -1;
		}

		if( pHttpclient->ctx )
		{
			SSL_CTX_free( pHttpclient->ctx );
			pHttpclient->ctx = NULL;
		}
		pHttpclient->used = 0;
	}
	m_ref--;

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	return NULL;
}
/***********************************************************************************************
* CONTENT: 与Http服务器建立连接
* PARAM:
	[in] ip: http服务器地址
	[in] port: http服务器端口
* RETURN:
	还回连接状态
* NOTE:
************************************************************************************************/
HANDLE sw_httpsclient_connect( unsigned long  ip, unsigned short port, int timeout )
{
	http_client_t* pHttpclient = NULL; 
	unsigned long unblock = 1;
	fd_set wset, rset;
	int i, n;
	int retcode;	
	unsigned int now;
	static bool first = true;	
	struct linger lingerOpt;	
	lingerOpt.l_onoff = 1;
    lingerOpt.l_linger = 0;
	int reuse = 1;
	X509*  server_cert =NULL;
	
    char*  str = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	BIO *in = NULL;
	X509 *https_cert = NULL;
	FILE *fp_ser;
	//FILE *fp_motive;
	FILE *re;
	unsigned long file_size = 0;
	char ser_buf[2*1024];
	char https_buf[2*1024];
	char cmd[64];
	int ret = -1;

	if(	m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( m_ref < 0 )
	{
		m_ref = 0;

		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
			m_all[i].skt = -1;
	}
	for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			pHttpclient = m_all + i;
			break;
		}
	}
	m_ref++;
	
	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	SWCOMMON_LOG_DEBUG("[HTTPCLIENT]0x%x, ip=%d, port=%d, timeout=%d, ref=%d\n",pHttpclient, ip, port, timeout, m_ref);
	
	if( pHttpclient == NULL )
		goto ERROR_EXIT;

	pHttpclient->isHttps = true;
	strlcpy(pHttpclient->version, "1.1", sizeof(pHttpclient->version));
	pHttpclient->ip = ip;
	pHttpclient->port = port;
	
	if(strlen(m_szCookies) > 0)
	  strlcpy(pHttpclient->sz_cookies, m_szCookies, sizeof(pHttpclient->sz_cookies));
	else
	  memset( pHttpclient->sz_cookies, 0, sizeof(pHttpclient->sz_cookies) );

	SSL_library_init();
	SSL_load_error_strings(); 

	/*Create a SSL_CTX by SSLv23_client_method(), (SSL Content Text)*/
  pHttpclient->ctx = SSL_CTX_new(SSLv23_client_method()); 
  if (pHttpclient->ctx == NULL) 
	{
		SWCOMMON_LOG_FATAL("SSL_CTX_new failed SSLv23_client_method!!!\n");
		goto ERROR_EXIT;
  }
	SWCOMMON_LOG_INFO("+++++++++ssl_ctx_new ok client+++++++++\n");
	
	SSL_CTX_set_verify(pHttpclient->ctx, SSL_VERIFY_NONE, NULL);

	/* 创建socket */
	pHttpclient->skt = sw_tcp_socket();
	if( pHttpclient->skt < 0 )
		goto ERROR_EXIT;
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( pHttpclient->skt, FIONBIO, &unblock ) < 0 )
		goto ERROR_EXIT;

	setsockopt(pHttpclient->skt , SOL_SOCKET, SO_REUSEADDR, 
				   (char *)&reuse, sizeof(reuse));	
	setsockopt( pHttpclient->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );

	now = sw_thrd_get_tick();
	if( first )
	{
		srand( now );
		sw_tcp_bind(pHttpclient->skt, INADDR_ANY, htons( (unsigned short) (30000 + rand()%26000 )));
		first = false;
	}
	/* 连接... */
	errno = 0;
	retcode = sw_tcp_connect( pHttpclient->skt, ip, port );
	if(retcode == 0)
		return pHttpclient;
	if(errno != 0 && errno != EINPROGRESS)
	{
		SWCOMMON_LOG_ERROR ("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
		goto ERROR_EXIT;
	}
	n=0;
WAIT:
	/* 等待连接成功 */
	if( (retcode = sw_tcp_select( pHttpclient->skt, &rset, &wset, NULL, timeout)) < 0 )
		goto ERROR_EXIT;

	if(retcode == 0)
	{
		SWCOMMON_LOG_ERROR("[HTTPCLIENT]select socket timeout!\n");
		goto ERROR_EXIT;
	}

	if( FD_ISSET( pHttpclient->skt, &rset ) )
	{
		char szBuf[16];
		int readsize = sw_tcp_recv( pHttpclient->skt, szBuf, sizeof(szBuf) );
		if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
		{
			if( ++n<3 )
				goto WAIT;
		}
	}
	else if( FD_ISSET( pHttpclient->skt, &wset ) )
	{
		int err;
		int len = sizeof(err);
#ifndef WIN32
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) 
#else
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0) 
#endif
		{
			SWCOMMON_LOG_ERROR ("[HTTPCLIENT]getsocketopt failed!\n");
			goto ERROR_EXIT;
		}

		if (err) 
		{
			printf ("[HTTPCLIENT] connect failed erron:%d!\n", err);
			goto ERROR_EXIT;
		}
		
		unblock = 0;
		sw_tcp_ioctl( pHttpclient->skt, FIONBIO, &unblock );

		pHttpclient->ssl = SSL_new(pHttpclient->ctx);
		if(pHttpclient->ssl == NULL )
		{
			SWCOMMON_LOG_FATAL("httpsclient SSL_new failed !!!\n");
			goto ERROR_EXIT;
		}

		SSL_set_fd(pHttpclient->ssl, pHttpclient->skt);
		if(SSL_connect(pHttpclient->ssl) == -1)
		{
			SWCOMMON_LOG_FATAL("httpsclient SSL_connect failed !!!\n");
			goto ERROR_EXIT;
		}
		str = (char*)SSL_get_cipher(pHttpclient->ssl);
		if(str)
		{
  		SWCOMMON_LOG_INFO("-------Connected with %s encryption\n",str);
		}
  	else
		{
			SWCOMMON_LOG_FATAL("httpsclient SSL_get_cipher failed !!!\n");
			goto ERROR_EXIT;
		}
		
		/* Get server's certificate (note: beware of dynamic allocation) - opt */
    server_cert = SSL_get_peer_certificate(pHttpclient->ssl);  
		if( server_cert )
		{
    	SWCOMMON_LOG_INFO("Server certificate:\n");
  		str = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
    	if(str)
			{
				SWCOMMON_LOG_INFO("\t subject: %s\n", str);
    		OPENSSL_free(str);
			}
    	else
			{
    		X509_free (server_cert);
				SWCOMMON_LOG_FATAL("Unkown certificate !!!\n");
				goto ERROR_EXIT;
			}

			str = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);
    	if(str)
			{
    		SWCOMMON_LOG_INFO("\t issuer: %s\n", str);
    		OPENSSL_free(str);
			}
			else
			{
    		X509_free (server_cert);
				SWCOMMON_LOG_FATAL("Unkown certificate !!!\n");
				goto ERROR_EXIT;
			}
#ifdef SUPPORT_OPENSSL
			fp_ser = fopen(SERVER_CERT,"wb+");
			PEM_write_X509(fp_ser,server_cert);
			fseek(fp_ser, 0, SEEK_END);
			file_size = ftell( fp_ser );
			printf("file_size = %ld\n",file_size);
			rewind( fp_ser );
			memset(ser_buf,0,sizeof(ser_buf));
			fread((void*)ser_buf,file_size,1,fp_ser);
			fclose(fp_ser);
#endif
#ifdef SUPPORT_TR069 
			printf("+++++++++++tr069+++++++++++++++\n");
			memset(cmd,0,sizeof(cmd));
			strlcpy(cmd,"cat /var/motive.pem", sizeof(cmd));
			ret = system(cmd);
			printf("ret ====== %d\n",ret);

			if( ret == 0 )
			{
				printf("find /var/motive.pem successful\n");
				in = BIO_new_file("/var/motive.pem","r");
				bSetTrustCert = true;
			}
			else
#endif
			{
				snprintf(cmd, sizeof(cmd), "cat %s",HTTPS_CERT);
				ret = system(cmd);
				if( ret == 0 )
				{
					in = BIO_new_file(HTTPS_CERT,"r");   
					bSetTrustCert = true;
				}
				else
				{
					bSetTrustCert = false;
				}
			}

			if( bSetTrustCert )
			{
				printf("++++++++++++set trusted cert++++++++++++++++\n");
				if(in==NULL)   
					printf("+++++++++++new file bio failed\n");  
				https_cert = PEM_read_bio_X509(in,NULL,NULL,NULL);  
#ifdef SUPPORT_OPENSSL
				re = fopen(RE_CERT,"wb+");
				PEM_write_X509(re,https_cert);
				fseek(re, 0, SEEK_END);
				file_size = ftell( re );
				printf("re_file_size = %ld\n",file_size);
				rewind( re );
				memset(https_buf,0,sizeof(https_buf));
				fread((void*)https_buf,file_size,1,re);
				fclose(re);
#endif
				p1 = strstr(ser_buf,"-----BEGIN CERTIFICATE-----");
				p2 = strstr(https_buf,"-----BEGIN CERTIFICATE-----");

				printf("p1:%s\n",p1);
				printf("p2:%s\n",p2);

				if( strcmp(p1,p2) == 0 )
					printf("verify pass\n");
				else
				{
					printf("verify failed\n");
					X509_free (server_cert);
					X509_free (https_cert);
					return NULL;
				}
				X509_free (https_cert);
			}

    	/* We could do all sorts of certificate verification stuff here before
			 deallocating the certificate. */
    		X509_free (server_cert);
		}

		return pHttpclient;
	}

ERROR_EXIT:
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if(pHttpclient )
	{
		if( pHttpclient->ssl )
		{
			SSL_shutdown(pHttpclient->ssl);
			SSL_free( pHttpclient->ssl);
			pHttpclient->ssl = NULL;
		}

		if( 0 <= pHttpclient->skt )
		{
			sw_tcp_close( pHttpclient->skt );
			pHttpclient->skt = -1;
		}

		if( pHttpclient->ctx )
		{
			SSL_CTX_free( pHttpclient->ctx );
			pHttpclient->ctx = NULL;
		}
		pHttpclient->used = 0;
	}
	m_ref--;

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	return NULL;
}


/***********************************************************************************************
* CONTENT: 断开与Http服务器建立的连接
* PARAM: 要断开的套接字  
* RETURN:
	连接服务器是否成功
* NOTE:
************************************************************************************************/
void sw_httpsclient_disconnect( HANDLE hHttpclient )
{
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient; 
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( pHttpclient )
	{		
		if( pHttpclient->ssl)
		{
			SSL_shutdown(pHttpclient->ssl);
			SSL_free(pHttpclient->ssl);
			pHttpclient->ssl  = NULL;
		}

		if( 0 <= pHttpclient->skt )
		{
			sw_tcp_close( pHttpclient->skt );
			pHttpclient->skt = -1;
		}

		if( pHttpclient->ctx)
		{
			SSL_CTX_free( pHttpclient->ctx );
			pHttpclient->ctx = NULL;
		}
		
		pHttpclient->used = 0;
		m_ref--;
	}
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
}


void httpsclient_shutdown( HANDLE hHttpclient )
{
	if(hHttpclient == NULL)
		return;
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient;
	if (pHttpclient->isHttps == false)
		sw_httpclient_shutdown(hHttpclient);
	else
	{
		if (pHttpclient->used && pHttpclient->ssl && pHttpclient->shutdown == false)
		{
			SSL_shutdown(pHttpclient->ssl);
			pHttpclient->shutdown = true;
		}
	}
}

/***********************************************************************************************
* CONTENT: 发送HTTP Request
* PARAM:
	[in] skt:建立连接后的套接字 
	[in] pMethod: 请求方式
	[in] pURL:请求的URL
	[in] pHost:主机名称
	[in] pAcceptType:接收的文件类型
	[in] pContentType:post 的内容类型
	[in] pContent:post 的内容
	[in] nLength: post 内容长度
	[in] pSOAPAction: soap action URI
	[in] pHttpAuth: 摘要认证信息
* RETURN:
	请求是否发送成功
* NOTE:
************************************************************************************************/

bool sw_httpsclient_request( HANDLE hHttpclient, char* pMethod,char* pURL,char *pHost,
			char *pAcceptType,char* pContentType,char* pContent,int nLength, int timeout,char* pSOAPAction,http_authentcation_t* pHttpAuth)
{
	nLength = (nLength>0) ? nLength : 0;
	char* pBuf = NULL;
	char *m_pReqBuf = NULL;
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient; 
	if (pHttpclient->used == 0)
		return false;
	if (pHttpclient->isHttps == false)
		return sw_httpclient_request(hHttpclient, pMethod, pURL, pHost, pAcceptType, pContentType, pContent, nLength, timeout, pSOAPAction, pHttpAuth);
	ahash_hex HA1;
	ahash_hex HA2 = "";
	ahash_hex m_response;
	char m_cNonce[48];
	char m_szNonceCount[9] = "00000001";
	int i =0;
	int ret = 0;
	int pos = 0;
	int reqlen = 6*8192 + nLength;

	if( pHttpclient == NULL || pMethod == NULL || pURL == NULL)
		return false;
	if(strcmp( pMethod, "GET" ) == 0)
		reqlen = 6*8192;
	m_pReqBuf = (char*)malloc(reqlen);
	if (m_pReqBuf == NULL)
		return false;
	pBuf =  m_pReqBuf;
	
	memset( m_cNonce, 0, sizeof(m_cNonce) );

	/* method */
	pos += strlcpy( &pBuf[pos], pMethod, pos < reqlen ? (reqlen-pos) : 0 );
	pos += strlcpy( &pBuf[pos], " ", pos < reqlen ? (reqlen-pos) : 0);
	
	/* url */
	sw_url_encode(pURL, &pBuf[pos]);
	pos += strlen(&pBuf[pos]);
	/* 版本 */
	pos += strlcpy( &pBuf[pos], " HTTP/1.1", pos < reqlen ? (reqlen-pos) : 0);
	
	/* host information */
	if(pHost && pHost[0] != '\0'  )
	{
		pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0,"\r\nHost: %s", pHost);
	}
	
	/* 接收类型 */
	pos += strlcpy( &pBuf[pos], "\r\nAccept: ", pos < reqlen ? (reqlen-pos) : 0);
	if( pAcceptType && pAcceptType[0] != '\0' )
	{
		pos += strlcpy( &pBuf[pos], pAcceptType, pos < reqlen ? (reqlen-pos) : 0 );
	}
	else
	{
		pos += strlcpy( &pBuf[pos], "*/*", pos < reqlen ? (reqlen-pos) : 0);
	}
	
	/* soap action */
	if ( pSOAPAction != NULL ) 
	{
		if( pSOAPAction[0] != '\0' )
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nSOAPAction: %s",pSOAPAction);
		else
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nSOAPAction:");		
	}
	/* 添加摘要认证信息 */
	if ( pHttpAuth != NULL ) 
	{
		if( strcmp(pHttpAuth->arithmetic,"Basic") ==0 )
		{
			char user_pass[128];
			char enc_user_pass[256];
			int  enc_length = 0;
			pos += strlcpy( &pBuf[pos], "\r\nauthorization: Basic ", pos < reqlen ? (reqlen-pos) : 0);
			snprintf(user_pass, sizeof(user_pass), "%s:%s", pHttpAuth->user_name ,pHttpAuth->user_pwd);
			enc_length = base64encode((const unsigned char*)user_pass,strlen(user_pass),(unsigned char*)enc_user_pass,sizeof(enc_user_pass)-1);
			enc_user_pass[enc_length] = '\0';
			pos += strlcpy( &pBuf[pos], enc_user_pass, pos < reqlen ? (reqlen-pos) : 0 );
		}
		else
		{
			// USERNAME
			pos += strlcpy( &pBuf[pos], "\r\nAuthorization: Digest username=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->user_name, pos < reqlen ? (reqlen-pos) : 0 );

			// REALM
			pos += strlcpy( &pBuf[pos], "\", realm=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->realm, pos < reqlen ? (reqlen-pos) : 0 );

			// NONCE
			pos += strlcpy( &pBuf[pos], "\", nonce=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->nonce, pos < reqlen ? (reqlen-pos) : 0 );

			// URI
			pos += strlcpy( &pBuf[pos], "\", uri=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->uri, pos < reqlen ? (reqlen-pos) : 0 );

			// QOP
			pos += strlcpy( &pBuf[pos], "\", qop=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->qop, pos < reqlen ? (reqlen-pos) : 0 );

			// NC
			pos += strlcpy( &pBuf[pos], "\", nc=", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_szNonceCount, pos < reqlen ? (reqlen-pos) : 0 );

			// CNONCE
			srand( (unsigned)time( NULL ) + rand()*2 );
			snprintf( m_cNonce, sizeof(m_cNonce), "%d", rand() ) ;
			//snprintf( m_cNonce, sizeof(m_cNonce), "%s", pHttpAuth->nonce );

			pos += strlcpy( &pBuf[pos], ", cnonce=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_cNonce, pos < reqlen ? (reqlen-pos) : 0 );

			// RESPONSE
			digest_calc_ha1(pHttpAuth->arithmetic, pHttpAuth->user_name, pHttpAuth->realm, pHttpAuth->user_pwd, pHttpAuth->nonce, m_cNonce, HA1);
			digest_calc_respose(HA1, pHttpAuth->nonce, m_szNonceCount, m_cNonce, pHttpAuth->qop, pMethod, pHttpAuth->uri, HA2, m_response);

			pos += strlcpy( &pBuf[pos], "\", response=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_response, pos < reqlen ? (reqlen-pos) : 0 );
		
			// OPAQUE
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\", opaque=\"%s\"", pHttpAuth->opaque);
		}
	}
	
	if( strcmp( pMethod, "POST" ) == 0 )
	{
		if( nLength >0 )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nContent-Length: %d", nLength );
		}
		else
		{
			pos += strlcpy( &pBuf[pos], "\r\nContent-Length: 0", pos < reqlen ? (reqlen-pos) : 0 );
		}
		if( nLength >0 )
		{ 
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nContent-Type: %s", pContentType );
		}
	}
	else if( strcmp( pMethod, "GET" ) == 0 )
	{
		if( nLength >0 )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nRange: bytes=%d-%s", nLength, pContent && strchr(pContent, '-')==NULL ? pContent : "" );
		}
		else if( pContent && strchr(pContent, '-') )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nRange: bytes=%s", pContent );
		}
	}
	/* 接收语言以及编码 */
	pos += strlcpy( &pBuf[pos],"\r\nAccept-language: zh-cn\r\nAccept-Encoding: gzip, deflate", pos < reqlen ? (reqlen-pos) : 0);
	
	/* user-agent */
	pos += strlcpy( &pBuf[pos],"\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; EIS iPanel 2.0;(ziva))", pos < reqlen ? (reqlen-pos) : 0);
	pos += strlcpy( &pBuf[pos],"\r\nPragma: no-cache", pos < reqlen ? (reqlen-pos) : 0);	
	pos += strlcpy( &pBuf[pos],"\r\nCache-Control: no-cache", pos < reqlen ? (reqlen-pos) : 0);

	if(pHttpclient->sz_cookies[0] != '\0')
	{
		if( (pos+strlen(pHttpclient->sz_cookies) < reqlen - 64) )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0,"\r\n%s", pHttpclient->sz_cookies);
		}
	}
	
	if(strcasecmp(pHttpclient->version, "1.0") == 0)
		pos += strlcpy( &pBuf[pos],"\r\nConnection: close\r\n\r\n", pos < reqlen ? (reqlen-pos) : 0);
	else
		pos += strlcpy( &pBuf[pos],"\r\nConnection: Keep-Alive\r\n\r\n", pos < reqlen ? (reqlen-pos) : 0);

	/* 发送content */
	if( strcmp( pMethod, "POST" ) == 0 )
	{
		if (pContent != NULL)
		{
			memcpy( &pBuf[pos], pContent, ((pos+nLength) < reqlen) ? nLength : 0);//肯定不会出现地址重叠
			pos += nLength;
		}
	}
	if (pos > reqlen)
	{
		free(m_pReqBuf);
		SWCOMMON_LOG_ERROR("%s header too long", pURL);
		return false;
	}
	i=0;
	reqlen = pos;
	while( i < reqlen)
	{
			/* 发送Http头 */
			ret = SSL_write(pHttpclient->ssl, m_pReqBuf+i, reqlen-i);			
			if( ret < 0 )
			{
				free(m_pReqBuf);
				return false;
			}
			else if( ret>0 && ( (i+ret) < reqlen) )
			{
				i += ret;
				continue;
			}
			else
			{
				free(m_pReqBuf);
				return true;
			}
	}
	free(m_pReqBuf);
	return false;
}

/***********************************************************************************************
* CONTENT: 发送HTTP Request
* PARAM:
	[in] skt:建立连接后的套接字 
	[in] pMethod: 请求方式
	[in] pURL:请求的URL
	[in] pHost:主机名称
	[in] pAcceptType:接收的文件类型
	[in] pFileName: 如果是传送文件时需要加上
	[in] pContentType:post 的内容类型
	[in] pContent:post 的内容
	[in] nLength: post 内容长度
	[in] pSOAPAction: soap action URI
	[in] pHttpAuth: 摘要认证信息
* RETURN:
	请求是否发送成功
* NOTE:
************************************************************************************************/

bool sw_httpsclient_request_ex( HANDLE hHttpclient, char* pMethod,char* pURL,char *pHost,
			char *pAcceptType,char *pFileName,char* pContentType,char* pContent,int nLength, int timeout,char* pSOAPAction,http_authentcation_t* pHttpAuth)
{
	nLength = (nLength>0) ? nLength : 0;
	char* pBuf = NULL;
	char *m_pReqBuf = NULL;
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient; 
	ahash_hex HA1;
	ahash_hex HA2 = "";
	ahash_hex m_response;
	char m_cNonce[48];
	char m_szNonceCount[9] = "00000001";
	int i =0;
	int ret = 0;
	int pos = 0;
	int reqlen = 6*8192 + nLength;

	if( pHttpclient == NULL || pMethod == NULL || pURL == NULL)
		return false;
	if(strcmp( pMethod, "GET" ) == 0)
		reqlen = 6*8192;
	m_pReqBuf = (char*)malloc(reqlen);
	if (m_pReqBuf == NULL)
		return false;
	pBuf =  m_pReqBuf;
	
	memset( m_cNonce, 0, sizeof(m_cNonce) );

	/* method */
	pos += strlcpy( &pBuf[pos], pMethod, pos < reqlen ? (reqlen-pos) : 0 );
	pos += strlcpy( &pBuf[pos], " ", pos < reqlen ? (reqlen-pos) : 0);
	
	/* url */
	sw_url_encode(pURL, &pBuf[pos]);
	pos += strlen(&pBuf[pos]);
	/* 版本 */
	pos += strlcpy( &pBuf[pos], " HTTP/1.1", pos < reqlen ? (reqlen-pos) : 0);
	
	/* host information */
	if(pHost && pHost[0] != '\0'  )
	{
		pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0,"\r\nHost: %s", pHost);
	}
	
	/* 接收类型 */
	pos += strlcpy( &pBuf[pos], "\r\nAccept: ", pos < reqlen ? (reqlen-pos) : 0);
	if( pAcceptType && pAcceptType[0] != '\0' )
	{
		pos += strlcpy( &pBuf[pos], pAcceptType, pos < reqlen ? (reqlen-pos) : 0 );
	}
	else
	{
		pos += strlcpy( &pBuf[pos], "*/*", pos < reqlen ? (reqlen-pos) : 0);
	}
	
	/* soap action */
	if ( pSOAPAction != NULL ) 
	{
		if( pSOAPAction[0] != '\0' )
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nSOAPAction: %s",pSOAPAction);
		else
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nSOAPAction:");		
	}
	/* 添加摘要认证信息 */
	if ( pHttpAuth != NULL ) 
	{
		if( strcmp(pHttpAuth->arithmetic,"Basic") ==0 )
		{
			char user_pass[128];
			char enc_user_pass[256];
			int  enc_length = 0;
			pos += strlcpy( &pBuf[pos], "\r\nauthorization: Basic ", pos < reqlen ? (reqlen-pos) : 0);
			snprintf(user_pass, sizeof(user_pass), "%s:%s", pHttpAuth->user_name ,pHttpAuth->user_pwd);
			enc_length = base64encode((const unsigned char*)user_pass,strlen(user_pass),(unsigned char*)enc_user_pass,sizeof(enc_user_pass)-1);
			enc_user_pass[enc_length] = '\0';
			pos += strlcpy( &pBuf[pos], enc_user_pass, pos < reqlen ? (reqlen-pos) : 0 );
		}
		else
		{
			// USERNAME
			pos += strlcpy( &pBuf[pos], "\r\nAuthorization: Digest username=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->user_name, pos < reqlen ? (reqlen-pos) : 0 );

			// REALM
			pos += strlcpy( &pBuf[pos], "\", realm=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->realm, pos < reqlen ? (reqlen-pos) : 0 );

			// NONCE
			pos += strlcpy( &pBuf[pos], "\", nonce=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->nonce, pos < reqlen ? (reqlen-pos) : 0 );

			// URI
			pos += strlcpy( &pBuf[pos], "\", uri=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->uri, pos < reqlen ? (reqlen-pos) : 0 );

			// QOP
			pos += strlcpy( &pBuf[pos], "\", qop=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], pHttpAuth->qop, pos < reqlen ? (reqlen-pos) : 0 );

			// NC
			pos += strlcpy( &pBuf[pos], "\", nc=", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_szNonceCount, pos < reqlen ? (reqlen-pos) : 0 );

			// CNONCE
			srand( (unsigned)time( NULL ) + rand()*2 );
			snprintf( m_cNonce, sizeof(m_cNonce), "%d", rand() ) ;
			//snprintf( m_cNonce, sizeof(m_cNonce), "%s", pHttpAuth->nonce );

			pos += strlcpy( &pBuf[pos], ", cnonce=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_cNonce, pos < reqlen ? (reqlen-pos) : 0 );

			// RESPONSE
			digest_calc_ha1(pHttpAuth->arithmetic, pHttpAuth->user_name, pHttpAuth->realm, pHttpAuth->user_pwd, pHttpAuth->nonce, m_cNonce, HA1);
			digest_calc_respose(HA1, pHttpAuth->nonce, m_szNonceCount, m_cNonce, pHttpAuth->qop, pMethod, pHttpAuth->uri, HA2, m_response);

			pos += strlcpy( &pBuf[pos], "\", response=\"", pos < reqlen ? (reqlen-pos) : 0);
			pos += strlcpy( &pBuf[pos], m_response, pos < reqlen ? (reqlen-pos) : 0 );
		
			// OPAQUE
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\", opaque=\"%s\"", pHttpAuth->opaque);
		}
	}
	
	if( strcmp( pMethod, "POST" ) == 0 )
	{
		if( nLength >0 )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nContent-Length: %d", nLength );
		}
		else
		{
			pos += strlcpy( &pBuf[pos], "\r\nContent-Length: 0", pos < reqlen ? (reqlen-pos) : 0 );
		}
		if( nLength >0 )
		{ 
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nContent-Type: %s", pContentType );
		}
		if(pFileName)
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nfilename: %s", pFileName );
		}
	}
	else if( strcmp( pMethod, "GET" ) == 0 )
	{
		if( nLength >0 )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nRange: bytes=%d-%s", nLength, pContent && strchr(pContent, '-')==NULL ? pContent : "" );
		}
		else if( pContent && strchr(pContent, '-') )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nRange: bytes=%s", pContent );
		}
	}
	/* 接收语言以及编码 */
	pos += strlcpy( &pBuf[pos],"\r\nAccept-language: zh-cn\r\nAccept-Encoding: gzip, deflate", pos < reqlen ? (reqlen-pos) : 0);
	
	/* user-agent */
	pos += strlcpy( &pBuf[pos],"\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; EIS iPanel 2.0;(ziva))", pos < reqlen ? (reqlen-pos) : 0);
	pos += strlcpy( &pBuf[pos],"\r\nPragma: no-cache", pos < reqlen ? (reqlen-pos) : 0);	
	pos += strlcpy( &pBuf[pos],"\r\nCache-Control: no-cache", pos < reqlen ? (reqlen-pos) : 0);

	if(pHttpclient->sz_cookies[0] != '\0')
	{
		if( (pos+strlen(pHttpclient->sz_cookies) < reqlen - 64) )
		{
			pos += snprintf( &pBuf[pos], pos < reqlen ? (reqlen-pos) : 0,"\r\n%s", pHttpclient->sz_cookies);
		}
	}
	
	if(strcasecmp(pHttpclient->version, "1.0") == 0)
		pos += strlcpy( &pBuf[pos],"\r\nConnection: close\r\n\r\n", pos < reqlen ? (reqlen-pos) : 0);
	else
		pos += strlcpy( &pBuf[pos],"\r\nConnection: Keep-Alive\r\n\r\n", pos < reqlen ? (reqlen-pos) : 0);

	/* 发送content */
	if( strcmp( pMethod, "POST" ) == 0 )
	{
		if (pContent != NULL)
		{
			memcpy( &pBuf[pos], pContent, ((pos+nLength) < reqlen) ? nLength : 0);//肯定不会出现地址重叠
			pos += nLength;
		}
	}
	if (pos > reqlen)
	{
		free(m_pReqBuf);
		SWCOMMON_LOG_ERROR("%s header too long", pURL);
		return false;
	}
	i=0;
	reqlen = pos;
	SWCOMMON_LOG_DEBUG("POST string = %s\n",m_pReqBuf);
	while( i < reqlen)
	{
			/* 发送Http头 */
			ret = SSL_write(pHttpclient->ssl, m_pReqBuf+i, reqlen-i);			
			if( ret < 0 )
			{
				free(m_pReqBuf);
				return false;
			}
			else if( ret>0 && ( (i+ret) < reqlen) )
			{
				i += ret;
				continue;
			}
			else
			{
				free(m_pReqBuf);
				return true;
			}
	}
	free(m_pReqBuf);
	return false;
}

/***********************************************************************************************
* CONTENT: 接收HTTP数据
* PARAM:
	[in] hHttpclient:建立连接后的套接字
	[in] pBuf:接收缓冲区
	[in] pHost:缓冲区大小
	[in] timeout:接收超时
* RETURN:
	接收到的数据长度
* NOTE:
************************************************************************************************/
int sw_httpsclient_recv_data( HANDLE hHttpclient, char* pBuf, int size, int timeout )
{
	if (hHttpclient == NULL || pBuf == NULL || size <= 0)
		return -1;
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient;
	if (pHttpclient->used == 0)
		return -1;
	if (pHttpclient->isHttps == false)
		return sw_httpclient_recv_data(hHttpclient, pBuf, size, timeout);

	if( pHttpclient && (pHttpclient->skt != -1) )
	{
		return SSL_read(pHttpclient->ssl, pBuf, size);			
	}
	return -1;
}

/** 
 * @brief 取得返回值
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpsclient_get_ret_code(char* pHeadBuf, int size)
{
	int i;
	char szBuffer[20];

	char* buf = pHeadBuf;
	if( pHeadBuf==NULL )
		return -1;

	if( !strncasecmp(buf, "http/", 5) )
	{
		buf += 5;
		while( (*buf>='0' && *buf<='9') || (*buf=='.') )
			buf++;
	}

	while( *buf==' ' )
		buf++;

	for(i=0; i<10 && *buf>='0' && *buf<='9'; i++, buf++)
	{
		szBuffer[i] = *buf;
	}
	szBuffer[i] = 0;
	return atoi( szBuffer );
}

/** 
 * @brief 取得负载长度
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
static int64_t str_int64( char* str )
{
	int64_t id = 0;
	while(*str==' ' || *str=='\t')
		str++;
	while( '0'<=*str && *str<='9' )
	{
		id = id*10 + (*str-'0');
		str++;
	}
	return id;
}

int64_t sw_httpsclient_get_content_size(char* pHeadBuf, int size)
{
	int i=0;
	while( i < size )
	{
		if( !memcmp(pHeadBuf+i, "Content-Length:", 15) )
		{
			return str_int64( pHeadBuf+i+15 );
		}
		while( i<size && pHeadBuf[i]!='\n' )
			i++;
		i++;
	}
	return 0;

}

/** 
 * @brief 得到http header的长度
 * 
 * @param pHeadBuf HTTP报文头接收
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpsclient_get_header_size(char* pHeadBuf, int size)
{
  	int i=0;
	int j=0;
	//句子编号
	int index =0;
	//句子长度
	int sentencelen=0;
	//所有句子的总长度
	int headerlen = 0;
	i=0;
	index =0;
	headerlen = 0;	
	while( i<size )
	{
		j=i;
		sentencelen = 0;
		for(;i<(size-1);i++)
		{
			if( pHeadBuf[i]=='\r' &&  pHeadBuf[i+1]=='\n' )
			{
				sentencelen = i-j+2;			
				break;
			}
		}
		
		i += 2;

		index++;
		//printf("Find  line %d length:%d i:%d size:%d\n",index,sentencelen,i,size);
		if( sentencelen >0 )
		{
			headerlen += sentencelen;
			if( sentencelen <= 2 )
			{
				break;
			}
		}
		else
			headerlen = size;
	}
	
	return headerlen;
}

/** 
 * @brief 注册cookies
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param cookies 预设置的cookies存放缓冲区
 * 
 * @return 
 */
int sw_httpsclient_register_cookies(HANDLE hHttpclient,char* cookies)
{

	if(hHttpclient == NULL)
	{
		memset(m_szCookies, 0, sizeof(m_szCookies));
		if(cookies)
			strlcpy(m_szCookies, cookies, sizeof(m_szCookies));
		return 0;
	}
  memset( ((http_client_t*)hHttpclient)->sz_cookies, 0, sizeof(((http_client_t*)hHttpclient)->sz_cookies) );
	if( cookies )
	{
		strlcpy( ((http_client_t*)hHttpclient)->sz_cookies, cookies, sizeof(((http_client_t*)hHttpclient)->sz_cookies));
	}
	return 0;
}

/** 
 * @brief 清空cookies
 * 
 * @param hHttpclient 已连接客户端的句柄
 * 
 * @return
 */
void sw_httpsclient_clear_cookies(HANDLE hHttpclient)
{
	if(hHttpclient == NULL)
	{
		memset(m_szCookies, 0, sizeof(m_szCookies));
	}
	else
	{
		memset( ((http_client_t*)hHttpclient)->sz_cookies,0,sizeof(((http_client_t*)hHttpclient)->sz_cookies));
	}
}


/** 
 * @brief 取得cookies
 * 
 * @param hHttpclient 已连接客户端的句柄
 * 
 * @return hHttpclient cookie 存储区
 */
char* sw_httpsclient_get_cookies(HANDLE hHttpclient)
{
	if(hHttpclient == NULL)
	{
		return m_szCookies;
	}
	return	((http_client_t*)hHttpclient)->sz_cookies;
}


/** 
 * @brief 设置HTTP版本号
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param version 预设置的HTTP版本号
 * 
 * @return 
 */
void sw_httpsclient_set_version(HANDLE hHttpclient, char* version)
{
	http_client_t* pHttpclient = ( http_client_t* )hHttpclient; 
	if(pHttpclient)
	{
		strlcpy(pHttpclient->version, version, sizeof(pHttpclient->version));
		
	}
}

