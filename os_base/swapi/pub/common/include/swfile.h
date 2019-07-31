#ifndef __SW_FILE_H__
#define __SW_FILE_H__

#if defined(__cplusplus)
extern "C"
{
#endif

    #define HTTP_CHUNK_SIZE -5

    typedef enum
    {/* 其定义规则为前4比特为扩展参数个数,后4比特为操作方式 */
		SW_FILE_IOCTL_NULL			= 0x00,
        SW_FILE_IOCTL_SET_TIMEOUT 	= 0x11,	/* 扩展参数为(int size) */
        SW_FILE_IOCTL_SET_COOKIE	= 0x12,	/* 扩展参数为(char *) */
		SW_FILE_IOCTL_GET_COOKIE	= 0x23,	/* 扩展参数为(char*, int size)  */
		SW_FILE_IOCTL_GET_FPATH		= 0x24,	/* 获取http实际访问的地址(有重定向的话返回重定向地置)扩展参数为(char*, int size) */
		SW_FILE_IOCTL_GET_CLIENT	= 0x15,	/* 获取http的client句柄,扩展参数(HANDLE *) */
		SW_FILE_IOCTL_GET_ETAG		= 0x26, /* 获取http文件的etag,扩展参数,(char*, int size) */
		SW_FILE_IOCTL_GET_CONTENT_ENCODING		= 0x27, /* 获取http文件的压缩方式,扩展参数,(char*, int size) */
		SW_FILE_IOCTL_GET_CONTENT_MD5		= 0x28, /* 获取http文件的MD5值,扩展参数,(char*, int size) */
        SW_FILE_IOCTL_MAX
    }swfile_ioctl_req_t;

    typedef struct swfile swfile_t;

    /*
     * @brief 打开文件
     * @param const char *name,文件的全路径(包括URL)
	 * @param const char *mode打开方式，"r", "w", "rw",对于ftp优先为读方式判断，对于http禁止写
     * @return swfile_t*,返回资源
     */
    swfile_t* sw_file_open( const char *name, const char *mode );


    /*
     * @brief 打开文件
     * @param const char *name,文件的全路径(包括URL)
	 * @param const char *mode打开方式，"r", "w", "rw",对于ftp优先为读方式判断，对于http禁止写
     * @param int timeout, 打开文件超时数
     * @param char* extension, http连接需要的扩展信息可以包含cookies,连接特性格式: '\0'是子串结束符，最后需要多余的一个结束符
				cookies=xx'\0',其cookies为xx=Cookie: ??
				connecttimes=2000,4000,8000'\0'，其建立连接时以2000ms，4000ms，8000ms序列去连接下载时以timeout为超时
				SOAPAction=xxx'\0',上传soap信息(长度为SOAPLen)
				ContentType=xxxx'\0', 当需要上报content是需要此字段
				ContentLength=xxxx'\0',需要上报的content内容长度------必须在Content前出现
				Content=xxxx'\0'内容字段(长度为ContentLength)-------此方式禁止seek方式下载
				Accept=xxxx'\0'
				authorization=xxxx'\0',摘要认证算法名称: md5, md5-sess,Basic
				authenname=xxxx'\0',摘要认证用户名
				authenpwd=xxxx'\0',摘要认证密码
				authenrealname=xxxx'\0',摘要认证realm name
				authennonce=xxxx'\0',摘要认证服务器随机产生的nonce返回串
				authenuri=xxxx'\0',摘要认证请求URL
				authenqop=xxxx'\0',摘要认证qop-value: "", "auth", "auth-int"
				authenopaque=xxxx'\0',摘要认证opaque value
				openpos=xxxx'\0',打开资源文件起始位置
				endpos=xxxx'\0',打开资源文件结束位置
				etag=xxxx'\0',被请求变量的实体值xx
				user-agent=xxxx'\0',用户代理xx=User-Agent: ??
				server-extension=xxxx'\0',服务器扩展字段,可以多个字段中间\r\n分开,最末尾字段不需要加,直接拼凑到request字段中
				extension=xxxx'\0',扩展字段,可以多个字段中间\r\n分开,最末尾字段不需要加,直接拼凑到request字段中,重定向时不拼接
				encoding=xx'\0', 用户可接收的文档编码方式xx=identity原始文件,gzip, deflate压缩
				language=xx'\0', 用户可接收的语言xx=zh-cn
				certpath=xx'\0', https的证书路径，如果有请确保正确
				certlen=%d'\0',	 https的证书内存缓存大小,没有这字段使用strlen(cerbuf);//先给出长度
				certaddr=%p'\0', https的证书内存缓存地址,如果有请确保正确,并确保连接中地址内存有效
				exclude-field=xx'\0' 那几个字段不需要在请求时带上去xx=[Accept-language,Accept-Encoding,User-Agent,Pragma,Cache-Control,Cookie]逗号分割
     * @return swfile_t*,返回资源
     */
    swfile_t* sw_file_open_ex( const char *name, const char *mode, int timeout, char* extension );

    /* 
     * @brief 关闭文件
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int sw_file_close( swfile_t *file );

    /*
     * @brief 读取数据
     * @param swfile_t *file, sw_file_open返回的资源
     * @param void *buf, 数据缓冲区
     * @param int size, 缓冲区大小
     */
    int sw_file_read( swfile_t *file, void *buf, int size );

    /*
     * @brief 写入数据
     * @param swfile_t *file, sw_file_open返回的资源
     * @param void *buf, 数据缓冲区
     * @param int size, 缓冲区大小
     */
    int sw_file_write( swfile_t *file, void *buf, int size );

    /* 
     * @brief 定位文件
     * @param swfile_t *file,sw_file_open返回的资源
     * @param int64_t offset,相对于origin的偏移量
     * @param int origin,起始位置
     * @return 成功 >= 0,失败-1
     */
    int sw_file_seek( swfile_t *file, int64_t offset, int origin );

    /* 
     * @brief 获取当前数据大小
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int64_t sw_file_tell( swfile_t *file );

    /* 
     * @brief 是否到了数据的结尾
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int sw_file_eof( swfile_t *file );

    /* 
     * @brief 获取单个字符
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int sw_file_getc( swfile_t *file );

    /* 
     * @brief 判断是否是可定位文件
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int sw_file_is_seekable( swfile_t *file );

    /* 
     * @brief 获取数据大小
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int64_t sw_file_get_size( swfile_t *file );

    /* 
     * @brief ioctl实现
     * @param swfile_t *file,sw_file_open返回的资源
     * @return -1表示失败
     */
	int sw_file_ioctl( swfile_t *file, int cmd,... );

    /* 
     * @brief 停止打开的文件连接
     *				这个多线程使用用于快速退出文件的读写操作,
     *				并且一般是在IPTV退出时使用
     * @param swfile_t *file,sw_file_open返回的资源
     * @return 成功 >= 0,失败-1
     */
    int sw_file_shutdown( swfile_t *file );

    /* 
     * @brief 下载文件,返回下载的数据大小和文件缓存,只下载小于“256M”的文件
     *		  需要调用方释放内filebuf
     *		  不能被打断下载
     *	
     * @param name文件路径
     * @param int timeout, 打开文件超时数
     * @param char* extension 参考sw_file_open_ex
     * @param filebuf下载申请的内存地址缓存
     * @return 下载到的数据长度,失败 < 0
     */
    int sw_file_download(const char *name, int timeout, char* extension, void **filebuf);

#if defined(__cplusplus)
}
#endif
#endif
