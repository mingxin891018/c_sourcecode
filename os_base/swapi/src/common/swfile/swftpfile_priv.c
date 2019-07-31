#include "swapi.h"
#include "swftpclient.h"
#include "swmem.h"
#include "swthrd.h"
#include "swtxtparser.h"
#include "swurl.h"
#include "swlog.h"
#include "swmutex.h"
#include "swcommon_priv.h"

#define MAX_FTPCLIENT_NUM		4

typedef struct tagftppara
{
	char ftpserver[16];
	char ftpport[8];
	char username[64];
	char userpswd[32];
	char ftppath[1024];
	char ftpfile[128];
	
}sftpparam_t;

/* FtpFile类 */
typedef struct
{
	/* 与ftpclient句柄 */
	HANDLE hftpclient;
	/* 文件大小 */
	int filesize;
	/*当前路径*/
	char curpath[1024];
	/*文件路径*/
	char ftppath[1024];
	/* 文件名 */
	char ftpfile[128];
	
}sftpfile_t;

static sftpfile_t m_all[MAX_FTPCLIENT_NUM];
static int m_ref = 0;
static HANDLE m_mutex = NULL;
/* find size */
static unsigned long findsize( char *buf, int len );
/* 分析url */
static bool ftp_url_parse( char *url, sftpparam_t *sftp_parm );

/** 
* @brief 初始化
* 
* @param url 指向文件下载的URL
* @param timeout 超时时间
* 
* @return 工作句柄,成功; 失败为NULL
*/
HANDLE sw_ftpfile_priv_init( char* url, int timeout )
{
	sftpfile_t *pftpfile = NULL;
	char *recvdata = NULL;
	sftpparam_t ftppara;
	unsigned long ip;
	unsigned short port;
	int i;
	char* p= NULL;

	if( m_mutex == NULL)
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	if( m_ref <= 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		m_ref = 0;
	}
	for( i=0; i<MAX_FTPCLIENT_NUM; i++ )
	{
		if( m_all[i].hftpclient == NULL )
		{
			pftpfile = m_all + i;
			break;
		}
	}
	m_ref++;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	if( pftpfile == NULL )
	{
		goto ERROR_EXIT;
	}

	/* 分析URL */
	memset( &ftppara, 0, sizeof(ftppara) );   //memset ftppara指针 何时释放?
	ftp_url_parse( url, &ftppara );
	if( ftppara.ftpfile[0] == '\0' )
	{
		goto ERROR_EXIT;
	}

	/* 连接FTP服务器 */
	memset( pftpfile, 0, sizeof(*pftpfile) );
	strlcpy( pftpfile->ftpfile, ftppara.ftpfile, sizeof(pftpfile->ftpfile) );	
	strlcpy( pftpfile->ftppath, ftppara.ftppath, sizeof(pftpfile->ftppath) );
	pftpfile->filesize = -1;
	if( !sw_txtparser_is_address(ftppara.ftpserver) )
	{
		struct hostent *h = gethostbyname(ftppara.ftpserver);  //
		if( h!= NULL && h->h_addr_list != NULL && h->h_addr_list[0] != NULL)
			memcpy(&ip, h->h_addr_list[0], sizeof(ip));//copy的是sizeof(unsigned long)
		else
		{
			goto ERROR_EXIT;
		}	
	}
	else
	{
		ip = inet_addr( ftppara.ftpserver );
	}
	
	port = htons( (unsigned short)atoi(ftppara.ftpport) );   //host序列  to  net 序列
	pftpfile->hftpclient = sw_ftpclient_connect( ip, port,timeout );
	if( pftpfile->hftpclient == NULL )
	{
		goto ERROR_EXIT;
	}
	
	/* 登录 */
	if( ! sw_ftpclient_login( pftpfile->hftpclient, ftppara.username, ftppara.userpswd, timeout ) )
	{
		goto ERROR_EXIT;
	}
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "SYST", NULL, timeout );

	memset(pftpfile->curpath,0,sizeof(pftpfile->curpath));
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "PWD", NULL, timeout );
	if( recvdata == NULL )
	{
		goto ERROR_EXIT;
	}
	if( atoi(recvdata) == 257 )    //功能：字符串转换成整型数,原型: int atoi(const char *nptr); 
								  //函数说明: 参数nptr字符串，如果第一个非空格字符不存在或者不是数字也不是正负号则返回零，
								 //否则开始做类型转换，之后检测到非数字或结束符 \0 时停止转换，返回整型数
	{	
		p =strchr(recvdata,'\"');
		if( p != NULL )
		{	
			p++;
			i=0;
			while( *p !='\"' && *p !='\0' && i < (int)sizeof(pftpfile->curpath)-1 )
			{
				pftpfile->curpath[i]=*p;
				i++;
				p++;
			}
			pftpfile->curpath[i]= '\0';
		}
	}

	/* 设置传输模式为二进制 */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "TYPE I", NULL, timeout );  //TYPE type说明文件类型,I表示图像（A略） 
	if( atoi(recvdata) != 200 )    //200表示服务器准备就绪
	{
		goto ERROR_EXIT;     
	}

	//m_ref++;
	return pftpfile;

ERROR_EXIT:
	SWCOMMON_LOG_ERROR( "init(\"%s\") failed\n", url );
	if( pftpfile )
	{
		if( pftpfile->hftpclient )
		{
			sw_ftpclient_disconnect( pftpfile->hftpclient, timeout );
		}
		if(m_mutex)
		{
			sw_mutex_lock(m_mutex);
		}
		memset( pftpfile, 0, sizeof(*pftpfile) );
        if(m_mutex)
		{
			sw_mutex_unlock(m_mutex);
		}
	}
	if(m_mutex)
	{
		sw_mutex_lock(m_mutex);
	}
	m_ref--;
	if(m_mutex)
	{
		sw_mutex_unlock(m_mutex);
	}
	return NULL;
}

/** 
* @brief 退出
* 
* @param hfile 工作句柄
* @param timeout 超时时间
*/
void sw_ftpfile_priv_exit( HANDLE hFile, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;

	if( pftpfile && pftpfile->hftpclient )
	{
		sw_ftpclient_disconnect( pftpfile->hftpclient, timeout );
		memset( pftpfile, 0, sizeof(*pftpfile) );
		m_ref--;
	}
}

/** 
* @brief 打印信息
*/
void sw_ftpfile_priv_print()
{
	int i;

	for( i=0; i<MAX_FTPCLIENT_NUM; i++ )
	{
		if( m_all[i].hftpclient )
		{
			SWCOMMON_LOG_INFO( "ftpfile(%d): %s\n", m_all[i].filesize, m_all[i].ftpfile );
		}
	}
}

/* 取得ftpclient句柄 */
HANDLE sw_ftpfile_priv_get_client( HANDLE hFile )
{
	return ((sftpfile_t *)hFile)->hftpclient;
}

/* 获取下载文件的大小 */
int sw_ftpfile_priv_get_size( HANDLE hFile, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	char *recvdata = NULL;
	int request = 0;
	
	char path[1024];
	if (snprintf(path, sizeof(path), "%s%s", pftpfile->ftppath, pftpfile->ftpfile) > sizeof(path))
	{
		SWCOMMON_LOG_ERROR("%s too long", path);
		return 0;
	}
	/* 支持SIZE 命令的服务器	 */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "SIZE", path, timeout );
	if( atoi(recvdata)==213 )     //213表示文件状态资讯
	{
		pftpfile->filesize = atol( recvdata+4 );
	}
	else
	{
		pftpfile->filesize = 0;
	}
	
	/* 请求下载文件(注：主要也是为了获取文件大小,有些服务器不支持SIZE命令) */
	request =  atoi( recvdata);
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "RETR", path, timeout );
	if( atoi(recvdata) <= 150 && request == 500 )   //小于150的应答有120,125,150,均表示RETR命令可识别 
													//如果不支持SIZE,会产生500,表示语法错误，不可识别的命令
	{
		pftpfile->filesize = findsize( recvdata, strlen(recvdata) );
	}

	return pftpfile->filesize;	
}


/** 
* @brief 获取下载文件的大小
* 
* @param hfile 工作句柄
* @param timeout 超时时间
* 
* @return 文件的大小,成功;否则, -1
*/

int sw_ftpfile_priv_get_file( HANDLE hFile, char *buf, int size, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	int totalsize = 0;
	int recvsize = 0 ,times = 0; //尝试连接的次数
	int maxsize = 0;
	int bufpos = 0;
	
	/* 没有调用sw_ftpfile_priv_get_size */
	if( pftpfile->filesize < 0 )
	{
		sw_ftpfile_priv_get_size( hFile, timeout );	
	}
     	
	totalsize = size < pftpfile->filesize ? size : pftpfile->filesize;
	if( totalsize <= 0 )
	{
		return 0;
	}
	
	memset( buf,0,totalsize );
	
	/* 下载数据 */
	while( bufpos < totalsize )
	{
		maxsize = 4096 < (totalsize - bufpos) ? 4096 : (totalsize - bufpos);
		if( ( recvsize = sw_ftpclient_recv_data( pftpfile->hftpclient,  buf+bufpos, maxsize, timeout ) ) > 0 )
		{
			bufpos += recvsize;
			times = 0;
			if( bufpos == totalsize )
			{
				break;
			}
		}
		else
		{
			sw_thrd_delay(10);
			times++;
			if( times >= 10 )
			{
				//printf("ftp unable read all the data\n");
				break;
			}
		}
	}
	
	return bufpos;		
}

/** 
* @brief 上传内容
* 
* @param hfile 工作句柄
* @param buf 指向存储文件的缓冲区
* @param size 缓冲区大小
* @param timeout 超时时间
* 
* @return 文件大小，成功；-1，失败  //return 0,成功; -1,失败
*/
int sw_ftpfile_priv_upload_file( HANDLE hFile, char *buf, int size, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	char *recvdata = NULL;
	int sendsize = 0,times = 0;
	int bufpos = 0;
	if(pftpfile->curpath[0] != '\0' && pftpfile->curpath[strlen(pftpfile->curpath)-1] == '/')   //修改当前路径
	{
		pftpfile->curpath[strlen(pftpfile->curpath)-1] = '\0'; 	
	}
	if (strlcat(pftpfile->curpath,pftpfile->ftppath, sizeof(pftpfile->curpath)) >= sizeof(pftpfile->curpath))
		SWCOMMON_LOG_ERROR("%s too long", pftpfile->curpath);

	/*改变当前的路径*/
    if (strlen(pftpfile->curpath) > 0)
    {
        recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "CWD", pftpfile->curpath, timeout );//???>250什么含义
        if( atoi( recvdata) > 250 )
            return 0;
    }
	/* 上传文件 */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "STOR", pftpfile->ftpfile, timeout );  //STOR存储文件
	if( atoi( recvdata) <=150 )
	{	
		while ( times < 10 )
		{
			if( size <= 1024 )
			{
				sendsize = sw_ftpclient_send_data( pftpfile->hftpclient, buf+bufpos,size, timeout );  //开始发数据
				if ( 0 < sendsize )
				{	
					bufpos += sendsize;
					break;
				}
				else 
				{
					sw_thrd_delay(100);
					times++;													
				}
			}
			else
			{
				sendsize = sw_ftpclient_send_data( pftpfile->hftpclient, buf+bufpos,1024, timeout );
				if ( sendsize > 0 )
				{
					bufpos += sendsize;
					size = size - sendsize;
					times = 0;		
				}
				else
				{
					sw_thrd_delay(100);
					times++;													
				}
			}
		}
	}
	return bufpos;	
}


/* find size */
static unsigned long findsize( char *buf, int len )  //???
{
	char *p = NULL;
	if( buf && atoi(buf)<=150 )
	{
		p = strchr( buf, '(' );  //应答格式:150 Opening BINARY mode data connection for bj.c (1010 bytes). 226 Transfer complete.
		if( p && (p+1) )
		{
			return atoi( p+1 ); //atoi(),函数说明: 参数nptr字符串，
			                   //如果第一个非空格字符不存在或者不是数字也不是正负号则返回零，否则开始做类型转换，
							   //之后检测到非数字或结束符 \0 时停止转换，返回整型数。 
		}
	}
	return 0;
}

/* 分析url */
static bool ftp_url_parse( char *url, sftpparam_t *sftp_parm  )
{
	sw_url_t surl;
	char* p = NULL;
	if( !url || strncmp( url,"ftp://",6)||!sftp_parm  )
	{
		return false;
	}

	sw_url_parse(&surl,url);      //ftp格式  ftp://用户名:密码@地址(或域名):端口/    如ftp://j00395:123456@210.42.35.26:8080/ 

	strlcpy(sftp_parm->ftpserver,surl.hostname,sizeof(sftp_parm->ftpserver));   //hostname 主机名（IP地址或域名）

	if( strlen(surl.user) !=0 )	
	{
		strlcpy(sftp_parm->username,surl.user,sizeof(sftp_parm->username)); //username 用户名
	}
	else
		strlcpy( sftp_parm->username,"anonymous", sizeof(sftp_parm->username));
	
	if( strlen(surl.pswd) !=0 )	
	{
		strlcpy(sftp_parm->userpswd,surl.pswd,sizeof(sftp_parm->userpswd));  //pswd 密码
	}

	if( surl.port == 0 )
	{
		strlcpy( sftp_parm->ftpport, "21", sizeof(sftp_parm->ftpport) );    //port 端口号   20用于控制连接，21用于数据连接,见P320
	}
	else	
	{
		snprintf(sftp_parm->ftpport, sizeof(sftp_parm->ftpport), "%d", ntohs(surl.port));
	}

	strlcpy( sftp_parm->ftppath, surl.path, sizeof(sftp_parm->ftppath) );   //path 文件路径
	p = strrchr(sftp_parm->ftppath,'/');
	if(p)
	{
		*(p+1) = '\0';
	}
	strlcpy( sftp_parm->ftpfile, surl.tail, sizeof(sftp_parm->ftpfile) );  //ftpfile 文件名


	return true;
}

