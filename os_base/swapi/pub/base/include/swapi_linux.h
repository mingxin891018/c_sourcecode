#ifndef __SWAPI_LINUX_H__
#define __SWAPI_LINUX_H__


#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <termios.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <net/if.h> 
#include <net/if_arp.h> 

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h> 
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <dlfcn.h>
#include <linux/reboot.h>
#include <linux/if_ether.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> //for offsetof
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <linux/if_packet.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define VA_ARG_N(	\
			_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
			_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,	\
			_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,	\
			_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,	\
			_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,	\
			_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,	\
			_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,	\
			_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,	\
			_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,	\
			_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,	\
			_101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
			_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,	\
			_121,_122,_123,_124,_125,_126,_127,_128,_129,_130,	\
			_131,_132,_133,_134,_135,_136,_137,_138,_139,_140,	\
			_141,_142,_143,_144,_145,_146,_147,_148,_149,_150,	\
			_151,_152,_153,_154,_155,_156,_157,_158,_159,_160,	\
			_161,_162,_163,_164,_165,_166,_167,_168,_169,_170,	\
			_171,_172,_173,_174,_175,_176,_177,_178,_179,_180,	\
			_181,_182,_183,_184,_185,_186,_187,_188,_189,_190,	\
			_191,_192,_193,_194,_195,_196,_197,_198,_199,_200,	\
			_201,_202,_203,_204,_205,_206,_207,_208,_209,_210, \
			_211,_212,_213,_214,_215,_216,_217,_218,_219,_220,	\
			_221,_222,_223,_224,_225,_226,_227,_228,_229,_230,	\
			_231,_232,_233,_234,_235,_236,_237,_238,_239,_240,	\
			_241,_242,_243,_244,_245,_246,_247,_248,_249,_250,	\
			_251,_252,_253,_254,_255,_256, N,...) N
#define VA_RSEQ_N()		256,255,254,253,252,251,250, \
		249,248,247,246,245,244,243,242,241,240, \
		239,238,237,236,235,234,233,232,231,230, \
		229,228,227,226,225,224,223,222,221,220, \
		219,218,217,216,215,214,213,212,211,210, \
		209,208,207,206,205,204,203,202,201,200,\
		199,198,197,196,195,194,193,192,191,190, \
		189,188,187,186,185,184,183,182,181,180, \
		179,178,177,176,175,174,173,172,171,170, \
		169,168,167,166,165,164,163,162,161,160, \
		159,158,157,156,155,154,153,152,151,150, \
		149,148,147,146,145,144,143,142,141,140, \
		139,138,137,136,135,134,133,132,131,130, \
		129,128,127,126,125,124,123,122,121,120, \
		119,118,117,116,115,114,113,112,111,110, \
		109,108,107,106,105,104,103,102,101,100, \
		99,98,97,96,95,94,93,92,91,90, \
		89,88,87,86,85,84,83,82,81,80, \
		79,78,77,76,75,74,73,72,71,70, \
		69,68,67,66,65,64,63,62,61,60, \
		59,58,57,56,55,54,53,52,51,50, \
		49,48,47,46,45,44,43,42,41,40, \
		39,38,37,36,35,34,33,32,31,30, \
		29,28,27,26,25,24,23,22,21,20, \
		19,18,17,16,15,14,13,12,11,10, \
		9,8,7,6,5,4,3,2,1,0
#define VA_NARG_(...) VA_ARG_N(__VA_ARGS__)
#define VA_NARGS(...) VA_NARG_(__VA_ARGS__,VA_RSEQ_N())
//fun(VA_NARGS(__VA_ARGS__), fmt,##__VA_ARGS__)	//如果超过256的话个数就不准确了

/**
 *@brief	这里size_t需要和INT_MAX比较其合法性，缓冲区合法后最终调用的是memcpy
 *@param	numberOfElementsDst目标缓存大小
 *			numberOfElementsSrc源的最大长度
 *			copy_amount需要拷贝的长度
 **/
void *__sw_memcpy(void *dest, size_t numberOfElementsDst, const void *src, size_t numberOfElementsSrc, size_t copy_amount);

/**
 *@brief	这里size_t需要和INT_MAX比较其合法性，缓冲区合法后最终调用的是memmove
 *@param	numberOfElementsDst目标缓存大小
 *			numberOfElementsSrc源的最大长度
 *			len需要移动的长度
 **/
void *__sw_memmove(void *dest, size_t numberOfElementsDst, const void *src, size_t numberOfElementsSrc, size_t len);

/**
 *@brief	这里size_t需要和INT_MAX比较其合法性，缓冲区合法后最终调用的是memset
 *@param	numberOfElementsDst目标缓存大小
 *			count需要设置为ch的长度
 **/
void *__sw_memset(void *dest, size_t numberOfElementsDst, int ch, size_t count);

/**
 *@brief	这里size_t需要和INT_MAX比较其合法性，缓冲区合法后最终调用的是memcmp
 *@param	numberOfElementsDst目标缓存大小
 *			numberOfElementsSrc源的最大长度
 *			len需要比较的长度
 **/
int __sw_memcmp(void *dest, size_t numberOfElementsDst, const void *src, size_t numberOfElementsSrc, size_t len);

size_t __sw_strlcpy(char *dest, size_t numberOfElementsDst, const char *src, size_t count);
size_t __sw_strlcat(char *dest, size_t numberOfElementsDst, const char *src, size_t count);

/**
 *@brief	这里size_t需要和INT_MAX比较其合法性
 *@param	numberOfElementsDst目标缓存大小
 *			startPos起始填充的目标缓冲位置
 *			count需要printf填充出的长度
 **/
int __sw_snprintf(void *dest, size_t numberOfElementsDst, size_t startPos, size_t count,
			 const char *format, ...) __attribute__ ((__format__ (__printf__, 5, 6)));
int __sw_vsnprintf(void *dest, size_t numberOfElementsDst, size_t startPos, size_t count,
			 const char *format,  va_list ap);

/**
 * c、 C、 s、 S和 [时, 必须提供字符的缓冲区大小作为参数,其紧跟在缓冲区地址参数后面,int型参数
 * vfscanf原始是%Ns的行为是如果输入的字符串大于等于N时,调用方需要预留一个额外的字节用于'\0'
 * sw_vfscanf这里不在需要调用方预留额外的字节
 * 支持的是OPENBSD开源格式而不是glibc的格式
 **/
int __sw_vfscanf(FILE *fp, const char *fmt0, va_list ap);
int __sw_fscanf(FILE *fp, const char *fmt, ...);
int __sw_sscanf(const char *str, const char *fmt, ...);
int __sw_scanf(const char *fmt, ...);
//__sw_scanf(str,"%s %c %d %[]", st SCAN_BUFSIZE(sizeof(st)), sc SCAN_BUFSIZE(sizeof(sc)), &sd, s_ SCAN_BUFSIZE(sizeof(s_)) SCAN_BUFSIZE(0));
int __sw_vscanf(const char *fmt, va_list ap);
int __sw_vsscanf(const char *str, const char *fmt, va_list ap);

/*
 * linux版本使用的是glibc里面的代码其vfscanf移植东西较多,这里可以强制进行fmt检测后在调用实际的接口
*/

#ifndef ANDROID
#ifdef SUPPORT_LIBC
size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);
#endif
#endif

#ifdef SUPPORT_SECURE
/*下面函数异常时打印异常，但不死机处理*/
#define SCAN_BUFSIZE(x) ,((void*)((int)(x)))	//转为指针类型好统一参数sizeof
#define sw_memcpy	__sw_memcpy
#define sw_memmove	__sw_memmove
#define sw_memset	__sw_memset
#define sw_memcmp	__sw_memcmp
#define sw_strlcat	__sw_strlcat
#define sw_strlcpy	__sw_strlcpy
#define sw_snprintf		__sw_snprintf
#define sw_vsnprintf 	__sw_vsnprintf
#define sw_vfscanf __sw_vfscanf
#define sw_vscanf __sw_vscanf
#define sw_vsscanf __sw_vsscanf
#define sw_scanf(fmt,...)	__sw_scanf(fmt,##__VA_ARGS__,((void*)((int)(0))))
#define sw_sscanf(str,fmt,...)	__sw_sscanf(str,fmt,##__VA_ARGS__,((void*)((int)(0))))
#define sw_fscanf(fp,fmt,...)	__sw_fscanf(fp,fmt,##__VA_ARGS__,((void*)((int)(0))))
#else/* ------------#ifdef SUPPORT_SECURE_LIBC-------------------- */
#define SCAN_BUFSIZE(x)	
#define sw_memcpy(dest,numberOfElementsDst,src,numberOfElementsSrc,copy_amount) 	\
		memcpy((char*)(dest),(char*)(src), (((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(numberOfElementsSrc) || (INT_MAX) < (size_t)(copy_amount) || (size_t)(numberOfElementsDst) < (copy_amount) || (size_t)(numberOfElementsSrc) < (copy_amount) ) ? 0 : (copy_amount)) )
#define sw_memmove(dest,numberOfElementsDst,src,numberOfElementsSrc,copy_amount) 	\
		memmove((char*)(dest),(char*)(src), (((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(numberOfElementsSrc) || (INT_MAX) < (size_t)(copy_amount) || (size_t)(numberOfElementsDst) < (size_t)(copy_amount) || (size_t)(numberOfElementsSrc) < (size_t)(copy_amount) ) ? 0 : (size_t)(copy_amount)) )
#define sw_memset(dest,numberOfElementsDst,ch,len) \
		memset((char*)(dest),(int)(ch), (((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(len) || (size_t)(numberOfElementsDst) < (size_t)(len) ) ? 0 : (len) ) ) 
#define sw_memcmp(dest,numberOfElementsDst,src,numberOfElementsSrc,len) 	\
		( ((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(numberOfElementsSrc) || (INT_MAX) < (size_t)(len) || (size_t)(numberOfElementsDst) < (len) || (size_t)((numberOfElementsSrc) < (len) )) ? 1 : memcmp(dest,src, len) )
#define sw_strlcat(dest,numberOfElementsDst,src,count)	\
		strlcat((char*)(dest),(char*)(src), (((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(count) ) ? 0 : ( ((numberOfElementsDst) > (count)) ? (count) : (numberOfElementsDst) ) ) )
#define sw_strlcpy(dest,numberOfElementsDst,src,count)	\
		strlcpy((char*)(dest),(char*)(src), (((INT_MAX) < (size_t)(numberOfElementsDst) || (INT_MAX) < (size_t)(count) ) ? 0 : ( ((numberOfElementsDst) > (count)) ? (count) : (numberOfElementsDst) ) ) )
#define sw_snprintf(dest,numberOfElementsDst,startPos,count,format,...)	\
		snprintf(&((char*)(dest))[(startPos)],(((INT_MAX) < (size_t)(numberOfElementsDst) || (size_t)(numberOfElementsDst)<=(size_t)(startPos) ) ? 0 : (((size_t)((numberOfElementsDst)-(startPos))>(size_t)(count)) ? (count) : ((numberOfElementsDst)-(startPos)))),format,##__VA_ARGS__)
#define sw_vsnprintf(dest,numberOfElementsDst,startPos,count,format,ap)	\
		vsnprintf(&((char*)(dest))[(startPos)],(((INT_MAX) < (size_t)(numberOfElementsDst) || (size_t)(numberOfElementsDst)<=(size_t)(startPos) ) ? 0 : (((size_t)((numberOfElementsDst)-(startPos))>(size_t)(count)) ? (count) : ((numberOfElementsDst)-(startPos)))),format,ap)
#define sw_scanf scanf
#define sw_sscanf sscanf
#define sw_fscanf fscanf
#define sw_vfscanf vfscanf
#define sw_vscanf vscanf
#define sw_vsscanf vsscanf
#endif/*------------#endif SUPPORT_SECURE_LIBC------------*/


#ifdef SUPPORT_SECURE
#define CLEAN_SENSITIVE_DATA(dst,ch,siz)	sw_memset(dst,siz,ch,siz)
#else/*------------#ifdef SUPPORT_SECURE------------*/
#define CLEAN_SENSITIVE_DATA(dst,ch,siz)	
#endif/*------------#endif SUPPORT_SECURE------------*/

#ifdef SUPPORT_SECURE_LOG
#define SENSITIVE_PRINT	false
#else
#define SENSITIVE_PRINT	true
#endif

#define sw_strdup(src) src == NULL ? NULL : strdup(src)

#ifdef __cplusplus
}
#endif

#endif /*end __SWAPI_LINUX_H__ */
