#include <stddef.h>
#include "swapi.h"
#include "swmutex.h"
#include "swurl.h"
#include "swlog.h"
#include "swtxtparser.h"
#include "swcommon_priv.h"
#include "swfile_priv.h"
#include "swftpclient.h"

#define MAX_PACKET_SIZE (1024 * 16)
#define SWFTPFILE_TIMEOUT_DEFAULT 5000

typedef struct _ftp_param
{
	char ftpserver[16];
	char ftpport[8];
	char username[64];
	char userpswd[32];
	char ftppath[1024];
	char ftpfile[128];
} ftp_param_t;

typedef struct swftpfile
{
    swfile_t impl;
    HANDLE h_ftpclient;
    char url[1024];

    int64_t filesize;
    int64_t curr_pos;
    int eof;
    int timeout;

	/* 文件大小 */
	/*当前路径*/
	char curpath[1024];
	/*文件路径*/
	char ftppath[1024];
	/* 文件名 */
	char ftpfile[128];
} swftpfile_t; 


/* find size */
static unsigned long FindSize(char *buf, int len);
/* 分析url */
static bool ftpurlParse(char *url, ftp_param_t *mPara);


static void sw_ftpfile_disconnect(swftpfile_t *ftp)
{
    if (ftp->h_ftpclient)
        sw_ftpclient_disconnect(ftp->h_ftpclient, 1000);
    ftp->h_ftpclient = NULL;
}


static int sw_ftpfile_connect(swftpfile_t *ftp, int read, int timeout)
{
	char *recvdata = NULL;
	ftp_param_t ftppara;
	unsigned long ip;
	unsigned short port;
	int i;
	char *p= NULL;

    sw_ftpfile_disconnect(ftp);

	/* 分析URL */
	memset(&ftppara, 0, sizeof(ftppara));
	ftpurlParse(ftp->url, &ftppara);
	if (ftppara.ftpfile[0] == '\0')
    {
        goto FAIL;
    }

	/* 连接FTP服务器 */
	strlcpy(ftp->ftpfile, ftppara.ftpfile, sizeof(ftp->ftpfile));	
	strlcpy(ftp->ftppath, ftppara.ftppath, sizeof(ftp->ftppath));
	ftp->filesize = -1;
	if (!sw_txtparser_is_address(ftppara.ftpserver))
	{
		struct hostent *h = gethostbyname(ftppara.ftpserver);
		if (h!= NULL && h->h_addr_list != NULL && h->h_addr_list[0] != NULL)
			memcpy(&ip, h->h_addr_list[0], sizeof(ip));//copy的是sizeof(unsigned long)
		else
		{
			SWCOMMON_LOG_ERROR("%s host ip is error!\n", __FUNCTION__);
            goto FAIL;
		}	
	}
	else
	{
		ip = inet_addr(ftppara.ftpserver);
	}

	port = htons((unsigned short)atoi(ftppara.ftpport));
	ftp->h_ftpclient = sw_ftpclient_connect(ip, port, timeout);
	if (ftp->h_ftpclient == NULL)
		goto FAIL;;
	
	/* 登录 */
	if (! sw_ftpclient_login(ftp->h_ftpclient, ftppara.username, ftppara.userpswd, timeout))
		goto FAIL;;
	recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "SYST", NULL, timeout);

	memset(ftp->curpath, 0, sizeof(ftp->curpath));
	recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "PWD", NULL, timeout);
	if (recvdata == NULL)
		goto FAIL;
	if (atoi(recvdata)==257)
	{	
		p = strchr(recvdata, '\"');
		if (p != NULL)
		{	
			p++;
			i = 0;
			while (*p != '\"' && *p != '\0' && i < (int)sizeof(ftp->curpath) - 1)
			{
				ftp->curpath[i] = *p;
				i++;
				p++;
			}
			ftp->curpath[i]= '\0';
		}
		SWCOMMON_LOG_INFO("(%s) PWD:%s\n", __FUNCTION__, ftp->curpath);
	}

	/* 设置传输模式为二进制 */
	recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "TYPE I", NULL, timeout);
	if (atoi(recvdata) != 200)
		goto FAIL;

    if (read)
    {
        char *recvdata = NULL;
        int request = 0;

        char path[1024];
        if (snprintf(path, sizeof(path), "%s%s", ftp->ftppath, ftp->ftpfile) >= (int)sizeof(path) )//文件名过长
        	goto FAIL;
        /* 支持SIZE 命令的服务器	 */
        recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "SIZE", path, timeout);
        if (atoi(recvdata)==213)
            ftp->filesize = atol(recvdata+4);
        else
            ftp->filesize = 0;

        /* 请求下载文件(注：主要也是为了获取文件大小, 有些服务器不支持SIZE命令) */
        request =  atoi(recvdata);
        recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "RETR", path, timeout);
        if (atoi(recvdata) <= 150 && request == 500)
            ftp->filesize = FindSize(recvdata, strlen(recvdata));
    }
    else
    {
        char *recvdata = NULL;
        if (ftp->curpath[0] != '\0' && ftp->curpath[strlen(ftp->curpath)-1] == '/')
        {
            ftp->curpath[strlen(ftp->curpath)-1] = '\0'; 	
        }
        strlcat(ftp->curpath, ftp->ftppath, sizeof(ftp->curpath));
        SWCOMMON_LOG_INFO("(%s) %s\n", __FUNCTION__, ftp->curpath);

        /*改变当前的路径*/
        if (strlen(ftp->curpath) > 0)
        {
            recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "CWD", ftp->curpath, timeout);
            if (atoi(recvdata) > 250)
                return 0;
        }
        /* 上传文件 */
        recvdata = sw_ftpclient_send_command(ftp->h_ftpclient, "STOR", ftp->ftpfile, timeout);
        if (atoi(recvdata) > 150)
        {
            goto FAIL;
        }
    }

	return 0;

FAIL:
	SWCOMMON_LOG_ERROR("%s FAILED! url:%s\n", __FUNCTION__, "not null");
    if (ftp->h_ftpclient)
    {
        sw_ftpclient_disconnect(ftp->h_ftpclient, timeout);
        ftp->h_ftpclient = NULL;
    }
	return -1;
}


static int sw_ftpfile_read(swfile_t *file, void *buf, int size)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    char *p_buf = buf;
    int nsize = size;
    int total_read_size = 0;
    int curr_read_size;

    while (nsize > 0)
    {
        curr_read_size = sw_ftpclient_recv_data(ftp->h_ftpclient, p_buf, nsize, ftp->timeout);
        if (curr_read_size > 0)
        {
            p_buf += curr_read_size;
            nsize -= curr_read_size;
            total_read_size += curr_read_size;
            ftp->curr_pos += curr_read_size;
        }
        else
        {
            if (ftp->eof != 1)
            {
                SWCOMMON_LOG_ERROR("(%s %p) can not receive data, EOF...\n", __FUNCTION__, ftp);
                ftp->eof = 1;
            }
            break;
        }
    }
    
    if (size > total_read_size)
        SWCOMMON_LOG_ERROR("(%s %p) read file failed! require:%d, real:%d\n", __FUNCTION__, ftp, size, total_read_size);

    return total_read_size;
}


static int sw_ftpfile_write(swfile_t *file, void *buf, int size)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    char *p_buf = buf;
    int nsize = size;
    int total_send_size = 0;
    int curr_send_size;
    int failed_count = 0;

    while (nsize > 0)
    {
        curr_send_size = sw_ftpclient_send_data(ftp->h_ftpclient, p_buf, nsize, ftp->timeout);
        if (curr_send_size > 0)
        {
            p_buf += curr_send_size;
            nsize -= curr_send_size;
            total_send_size += curr_send_size;
            ftp->curr_pos += curr_send_size;
            failed_count = 0;
        }
        else
        {
            failed_count++;
            if (failed_count > 5)
                break;
        }
    }

    if (size > total_send_size)
        SWCOMMON_LOG_ERROR("(%s %p) send file failed! require:%d, real:%d\n", __FUNCTION__, ftp, size, total_send_size);

    return total_send_size;
}


static int sw_ftpfile_seek(swfile_t *file, int64_t offset, int origin)
{
    SW_UNUSED(file);
    SW_UNUSED(offset);
    SW_UNUSED(origin);
    return -1;
}


static int64_t sw_ftpfile_tell(swfile_t *file)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    SWCOMMON_LOG_DEBUG("(%s %p) %lld, %p\n", __FUNCTION__, ftp, ftp->curr_pos, ftp);
    return ftp->curr_pos;
}


static int sw_ftpfile_eof(swfile_t *file)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    return ftp->eof;
}


static int sw_ftpfile_getc(swfile_t *file)
{
    char c = 0;
    if (sw_ftpfile_read(file, &c, 1) == 1)
        return c;
    return EOF;
}


static int sw_ftpfile_is_seekable(swfile_t *file)
{
    SW_UNUSED(file);
    return -1;
}


static int64_t sw_ftpfile_get_size(swfile_t *file)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    return ftp->filesize;
}


static int sw_ftpfile_close(swfile_t *file)
{
    swftpfile_t *ftp = C2P(file, impl, swftpfile_t);
    SWCOMMON_LOG_INFO("%s %p\n", __FUNCTION__, ftp);
    sw_ftpfile_disconnect(ftp);
    free(ftp);
    return 0;
}


swfile_t* sw_ftpfile_open(const char *name, const char *mode)
{
    swftpfile_t *ftp = NULL;

    ftp = (swftpfile_t *)malloc(sizeof(swftpfile_t));
    if (ftp == NULL)
        goto FAIL;
    memset(ftp, 0, sizeof(swftpfile_t));

    SWCOMMON_LOG_INFO("%s %p\n", __FUNCTION__, ftp);

    strlcpy(ftp->url, name, sizeof(ftp->url));

    ftp->timeout = SWFTPFILE_TIMEOUT_DEFAULT;

    if (mode == NULL || strchr(mode, 'r') != NULL)
    {
        if (sw_ftpfile_connect(ftp, 1, 2000) != 0)
            goto FAIL;
    }
    else if (strchr(mode, 'w') != NULL)
    {
        if (sw_ftpfile_connect(ftp, 0, 2000) != 0)
            goto FAIL;
    }
    else
    {
        SWCOMMON_LOG_ERROR("(%s) unknow mode:%s\n", __FUNCTION__, mode);
        goto FAIL;
    }

    ftp->impl.read  = sw_ftpfile_read;
    ftp->impl.write = sw_ftpfile_write;
    ftp->impl.seek  = sw_ftpfile_seek;
    ftp->impl.tell  = sw_ftpfile_tell;
    ftp->impl.eof   = sw_ftpfile_eof;
    ftp->impl.agetc = sw_ftpfile_getc;
    ftp->impl.is_seekable = sw_ftpfile_is_seekable;
    ftp->impl.get_size    = sw_ftpfile_get_size;
    ftp->impl.close       = sw_ftpfile_close;

    return &ftp->impl;

FAIL:
    if (ftp != NULL)
    {
        sw_ftpfile_disconnect(ftp);
        free(ftp);
    }
    return NULL;
}

swfile_t* sw_ftpfile_open_ex(const char *name, const char *mode, int timeout)
{
    swftpfile_t *ftp = NULL;

    ftp = (swftpfile_t *)malloc(sizeof(swftpfile_t));
    if (ftp == NULL)
        goto FAIL;
    memset(ftp, 0, sizeof(swftpfile_t));

    SWCOMMON_LOG_INFO("%s %p\n", __FUNCTION__, ftp);

    strlcpy(ftp->url, name, sizeof(ftp->url));

    ftp->timeout = timeout;
    if (mode == NULL || strchr(mode, 'r') != NULL)
    {
		if (sw_ftpfile_connect(ftp, 1, ftp->timeout) != 0)
		    goto FAIL;
	}
    else if (strchr(mode, 'w') != NULL)
    {
        if (sw_ftpfile_connect(ftp, 0, ftp->timeout) != 0)
            goto FAIL;
    }
    else
    {
        SWCOMMON_LOG_ERROR("(%s) unknow mode:%s\n", __FUNCTION__, mode);
        goto FAIL;
    }

    ftp->impl.read  = sw_ftpfile_read;
    ftp->impl.write = sw_ftpfile_write;
    ftp->impl.seek  = sw_ftpfile_seek;
    ftp->impl.tell  = sw_ftpfile_tell;
    ftp->impl.eof   = sw_ftpfile_eof;
    ftp->impl.agetc = sw_ftpfile_getc;
    ftp->impl.is_seekable = sw_ftpfile_is_seekable;
    ftp->impl.get_size    = sw_ftpfile_get_size;
    ftp->impl.close       = sw_ftpfile_close;

    return &ftp->impl;

FAIL:
    if (ftp != NULL)
    {
        sw_ftpfile_disconnect(ftp);
        free(ftp);
    }
    return NULL;

}


/* find size */
static unsigned long FindSize(char *buf, int len)
{
	char *p = NULL;
	if (buf && atoi(buf)<=150)
	{
		p = strchr(buf, '(');
		if (p && (p+1))
		{
			return atoi(p+1);
		}
	}
	return 0;
}


/* 分析url */
static bool ftpurlParse(char *url, ftp_param_t *mPara)
{
	sw_url_t surl;
	char* p = NULL;
	if (!url || strncmp(url, "ftp://", 6)||!mPara)
		return false;

	sw_url_parse(&surl, url);

	strlcpy(mPara->ftpserver, surl.hostname, sizeof(mPara->ftpserver));
	
	if (strlen(surl.user) !=0)	
		strlcpy(mPara->username, surl.user, sizeof(mPara->username));
	else
		strlcpy(mPara->username, "anonymous", sizeof(mPara->username));
	
	if (strlen(surl.pswd) !=0)	
		strlcpy(mPara->userpswd, surl.pswd, sizeof(mPara->userpswd));

	if (surl.port == 0)
		strlcpy(mPara->ftpport, "21", sizeof(mPara->ftpport));
	else	
		snprintf(mPara->ftpport, sizeof(mPara->ftpport), "%d", ntohs(surl.port));

	strlcpy(mPara->ftppath, surl.path, sizeof(mPara->ftppath));
	p= strrchr(mPara->ftppath, '/');
	if (p)
		*(p+1) = '\0';
	strlcpy(mPara->ftpfile, surl.tail, sizeof(mPara->ftpfile));

	SWCOMMON_LOG_INFO("(%s) %s %s \n", __FUNCTION__, mPara->ftppath, mPara->ftpfile);
	return true;
}

