/**
 * @file swftpclient.h
 * @brief FTP函数接口
 * @author ...
 * @date 2007-09-06
 * @history
 * @2010-8-5 xusheng modify the format
 */


#include "swapi.h"
#include "swftpclient.h"
#include "swmem.h"
#include "swthrd.h"
#include "swtcp.h"
#include "swcommon_priv.h"

#define MAX_FTPFILE_NUM		4

static sftpclient_t m_all[MAX_FTPFILE_NUM];
static int m_ref = 0;

/** 
* @brief ：与服务器建立连接
* 
* @param ip 服务器的ip (ip和port均为网络字节序)
* @param port 连接端口
* @param timeout 超时时间
* 
* @return 工作句柄
*/

HANDLE sw_ftpclient_connect( unsigned long ip, unsigned short port, int timeout )
{
	sftpclient_t *pftpclient = NULL;
	unsigned long unblock = 1;
	fd_set rset;
	int i;

	if( m_ref <= 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_FTPFILE_NUM; i++ )
		{	
			m_all[i].cmdskt = m_all[i].datskt = -1;
		}
		m_ref = 0;
	}

	for( i=0; i<MAX_FTPFILE_NUM; i++ )
	{
		if( m_all[i].cmdskt < 0 )
		{
			pftpclient = m_all + i;  	//使pftpclient指向空闲的XXX
			break;
		}
	}
	if( pftpclient == NULL )
		goto ERROR_EXIT;

	memset( pftpclient, 0, sizeof(*pftpclient) );
	pftpclient->cmdskt = -1;
	pftpclient->datskt = -1;
	pftpclient->ip = ip;
	pftpclient->cmdport = port;		//为什么不给dataport赋值
	pftpclient->type = 1;

	/* 创建socket */
	pftpclient->cmdskt = sw_tcp_socket();
	if( pftpclient->cmdskt < 0 )
	{
		goto ERROR_EXIT;
	}
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( pftpclient->cmdskt, FIONBIO, &unblock ) < 0 )
	{
		goto ERROR_EXIT;
	}
	/* 连接... */
	sw_tcp_connect( pftpclient->cmdskt, ip, port );
	/* 等待连接成功 */
	if( sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout ) < 0 )
	{
		goto ERROR_EXIT;
	}

	m_ref++;
	return pftpclient;

ERROR_EXIT:

	if( pftpclient != NULL && 0 <= pftpclient->cmdskt )
	{
		sw_tcp_close( pftpclient->cmdskt );
		pftpclient->cmdskt = -1;
	}

	return NULL;
}

/** 
* @brief 断开与ftp服务器的连接
* 
* @param hftp 工作句柄
* @param timeout 超时时间
*/
void sw_ftpclient_disconnect( HANDLE hftp, int timeout )
{
	int ret = 0;
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;

	if( pftpclient && 0 <= pftpclient->cmdskt )
	{
		/* 关闭数据端口的连接 */
		if( 0 <= pftpclient->datskt )
		{	
			/* 等待数据传输完毕 */
			while( ! pftpclient->type && 0 < timeout )
			{	
				ret = atoi( sw_ftpclient_send_command(hftp,NULL,NULL,timeout) );
				if( ret == 226 || ret == 426 )     //226是???
				{
					break;
				}
				else
				{
					sw_thrd_delay(500);
					timeout -= 500;
				}
			}

			sw_tcp_close( pftpclient->datskt );
			pftpclient->datskt = -1;
		}
		/* 关闭指令连接 */
		sw_tcp_close( pftpclient->cmdskt );
		pftpclient->cmdskt = -1;
		m_ref--;
	}
}


/** 
* @brief 登入ftp服务器,建立数据端口连接
* 
* @param hftp 工作句柄
* @param username 用户名
* @param password 密码
* @param timeout 超时时间
* 
* @return true,成功;否则, false
*/
bool sw_ftpclient_login( HANDLE hftp, char* username, char* password, int timeout )
{
	unsigned long unblock = 1;
	char *ptr, *recvdata;
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset, wset;
    int ret;

	/* 接收服务器的数据 */
	recvdata = sw_ftpclient_send_command( hftp, NULL, NULL, timeout );
	if( atoi(recvdata) != 220 )     //这里的220,和下面的331,230具有什么含义?  220表示:新用户服务准备好了 
	{
		return false;
	}

	/* 检测用户名 */
	recvdata = sw_ftpclient_send_command( hftp, "USER", username, timeout );
	if( atoi(recvdata) != 331 && atoi(recvdata ) != 230 )  //331表示:用户名正确，需要口令；230表示：用户登录
	{
		return false;
	}
	/* 需要密码 */
	if( atoi( recvdata ) != 230 )    //recvdata==331:USER正确，需要PASS.
	{
		recvdata = sw_ftpclient_send_command( hftp, "PASS", password, timeout );
		if( atoi( recvdata) != 230 )   //recvdata==230:USER登录了
		{
			return false;
		}
	}
	/* 被动模式 */
	recvdata = sw_ftpclient_send_command( hftp,"PASV", NULL, timeout );

	/* 获取数据端口 */
	ptr = strrchr( recvdata, ',' ) ;
	if( ptr )        //GOOD！ 例如：140.252.12.34.4.150  
	{
		*ptr = '\0';
	     pftpclient->datport = atoi( ptr + 1 );
	     ptr = strrchr( recvdata, ',' );

		 if( ptr )
		 {
	         *ptr = '\0';
	         pftpclient->datport += atoi( ptr + 1 ) * 256;
		 }
	}

	/* 创建数据连接 */
	pftpclient->datskt = sw_tcp_socket();
	if( pftpclient->datskt < 0 )
	{
		goto ERROR_EXIT;
	}
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( pftpclient->datskt, FIONBIO, &unblock ) < 0 )
	{
		goto ERROR_EXIT;
	}
	/* 连接... */
	sw_tcp_connect( pftpclient->datskt, pftpclient->ip, htons(pftpclient->datport) );
	/* 等待连接成功 */
    ret = sw_tcp_select( pftpclient->datskt, &rset, &wset, NULL, timeout );
    if (ret < 0)
	{
		goto ERROR_EXIT;
	}
    else if (FD_ISSET(pftpclient->datskt, &rset))
	{
		goto ERROR_EXIT;
	}

	return true;

ERROR_EXIT:
	SWCOMMON_LOG_ERROR( "connect ftp data port failed !\n" );
	if( 0 <= pftpclient->datskt )
	{
		sw_tcp_close( pftpclient->datskt );
		pftpclient->datskt = -1;
	}

	return false;
}

/** 
* @brief 从ftp服务器数据端口接收数据
* 
* @param hftp 工作句柄
* @param buf 接收缓冲
* @param size 缓冲大小
* @param timeout 超时时间
* 
* @return 接收大小
*/
int sw_ftpclient_recv_data( HANDLE hftp, char *buf, int size, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset;

	/* 检测状态 */
	if( sw_tcp_select( pftpclient->datskt, &rset, NULL, NULL, timeout ) < 0 )
	{
		return 0;
	}

	/* 接收数据 */
	if( FD_ISSET(pftpclient->datskt, &rset) )
	{
		return sw_tcp_recv( pftpclient->datskt, buf, size );
	}
	else
	{
		return 0;
	}
}

/** 
* @brief 向ftp服务器数据端口发送数据
* 
* @param hftp 工作句柄
* @param buf 发送缓冲
* @param size 缓冲大小
* @param timeout 超时时间
* 
* @return 发送大小
*/
int sw_ftpclient_send_data( HANDLE hftp, char *buf, int size, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set wset;

	/* 检测状态 */
	if( sw_tcp_select( pftpclient->datskt, NULL, &wset, NULL, timeout ) < 0 )
	{
		return -1;
	}
	
	/* 发送数据 */
	if( FD_ISSET( pftpclient->datskt, &wset ) )
	{
		return sw_tcp_send( pftpclient->datskt, buf, size );
	}
	else
	{
		return 0;
	}
}


/** 
* @brief 向ftp服务器发送命令
* 
* @param hftp 工作句柄
* @param type 命令类型
* @param command 命令
* @param timeout 超时时间
* 
* @return 服务器的回复
*/
char* sw_ftpclient_send_command( HANDLE hftp, char *type,  char* command, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset, wset;
	char ftpcmd[2048];

	char return_end[8] = "\0";

	/* 清空回答 */
	memset( pftpclient->response, 0, sizeof(pftpclient->response) );
	memset(return_end, 0, sizeof(return_end));

	/* 构造ftp命令 */
	memset( ftpcmd,0 , sizeof(ftpcmd) );
	if( type )
	{
		if( command )
		{
			snprintf( ftpcmd, sizeof(ftpcmd), "%s %s\r\n", type, command );
		}
		else
		{
			snprintf( ftpcmd, sizeof(ftpcmd), "%s\r\n", type );
		}

		/* 检测状态 */
		sw_tcp_select( pftpclient->cmdskt, NULL, &wset, NULL, timeout );

		/* 发送数据 */
		if( FD_ISSET( pftpclient->cmdskt, &wset ) )
		{
			sw_tcp_send( pftpclient->cmdskt, ftpcmd,strlen(ftpcmd));
		}
	}

	/* 检测状态 */
	sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout );

	/* 接收回答 */
	if( FD_ISSET( pftpclient->cmdskt, &rset ) )
	{
		if(sw_tcp_recv( pftpclient->cmdskt, pftpclient->response, sizeof(pftpclient->response)-1 ) <= 0)
		{
			return pftpclient->response;
		}
		strncpy(return_end, pftpclient->response, 3);   //copy 应答的前3个字符，如：231//已经对return_end清0了
		strlcat(return_end, " ", sizeof(return_end));//加个空格

		while(strstr(pftpclient->response, return_end) == NULL)   //改条件为"\r\n"时结束
		{

			int return_bytes;
			char ftp_return_cmd[512] = "\0";
			memset( ftp_return_cmd, 0, sizeof(ftp_return_cmd));

			/* 检测状态 */
			if(	sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout ) <= 0 )
			{
				break;
			}
			/* 接收回答 */
			if( FD_ISSET( pftpclient->cmdskt, &rset ) )
			{
				if(( return_bytes = sw_tcp_recv( pftpclient->cmdskt, ftp_return_cmd, sizeof(ftp_return_cmd)-1 )) > 0)
				{
					int len = strlcat(pftpclient->response, ftp_return_cmd, sizeof(pftpclient->response));
					if (len >= (int)sizeof(pftpclient->response))//已经无法在接受多余字符了
						break;
				}
				else
				{
					break;
				}
			}

		}
	}

	if( type && strncmp(type,"STOR",4) == 0 && atoi(pftpclient->response) != 550  )
	{
		pftpclient->type = 0;
	}

	return pftpclient->response;
}
