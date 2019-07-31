/** 
 * @file swurl.h
 * @brief 定义一个URL分析功能接口
 * @author sunniwell
 * @date 2005-01-05 created 
 */

#ifndef __SWURL_H__
#define __SWURL_H__


#ifdef __cplusplus
extern "C"
{
#endif
	
typedef struct _sw_url
{
    /** 
     * @brief URL头 '// :'之前的一个词
     */
	char head[32];	
	char user[32];
	char pswd[32];
	/** 
	 * @brief URL 域名信息(可能是IP)
	 */
	char hostname[256];
	/** 
	 * @brief has ipv4 IP
	 */
	bool has_ipv4;
	/** 
	 * @brief has ipv6 IP
	 */
	bool has_ipv6;
	/** 
	 * @brief URL指向的IP，网络字节序
	 */
	uint32_t ip;
	/** 
	 * @brief URL指向的IP，ipv6
	 */
	struct in6_addr ipv6;
	/** 
	 * @brief URL端口，网络字节序
	 */
	uint16_t port;
	/** 
	 * @brief URL path
	 */
	char path[1024];		
	/** 
	 * @brief URL最后一个词
	 */
	char tail[256];
	/** 
	 * @brief URL最后一个词的后缀
	 */
	char suffix[32];
    /**
     * @brief hostname[:port]
     */
	char host[256];
}sw_url_t;

/** 
 * @brief 分析URL,
 * 
 * @param dst 指向分析后的结果
 * @param url 指向的源URL
 * 
 * @return 0表示分析成功,-1分析失败
 */
int sw_url_parse(sw_url_t* dst, const char* url);

/** 
 * @brief 取得URL中的参数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * @param value 指向的参数值
 * @param valuesize 指向参数值的长度
 * 
 * @return 成功,返回指向的参数值; 否则,返回NULL
 */
char* sw_url_get_param_value( const char* url, const char* name, char *value, int valuesize );

/** 
 * @brief 从URL中提取参数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * 
 * @return 成功,返回指向的参数值; 否则,返回NULL
 */
char* sw_url_get_param(char* url,char* name);

/** 
 * @brief 从URL中提取整数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * 
 * @return 成功,返回参数的整数值; 否则,返回-1
 */
int sw_url_get_param_int(const char* url, const char* name);

/** 
 * @brief 从URL中提取path
 * 
 * @param url 指向的源URL
 * @param path 缓冲区
 * @param size 缓冲区长度
 * 
 * @return 返回缓冲区地址
 */
char *sw_url_get_path(const char *url, char *path, int size);

/** 
 * @brief 从URL中提取header
 * 
 * @param url 指向的源URL
 * @param header 缓冲区
 * @param size 缓冲区长度
 * 
 * @return 返回缓冲区地址
 */
char *sw_url_get_header(const char *url, char *header, int size);

/** 
 * @brief 对URL进行编码
 * 
 * @param in 指向的输入字符串
 * @param out 指向的输出字符串
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int sw_url_encode(const char* in,char* out);

/** 
 * @brief 对URL进行编码
 * 
 * @param in 指向的输入字符串
 * @param out 指向的输出字符串
 * @param outsize 指向的输出字符串最大长度
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int sw_url_encode_ex(const char* in, char* out, int outsize);

#ifdef __cplusplus
}
#endif

#endif /* __SWURL_H__ */
