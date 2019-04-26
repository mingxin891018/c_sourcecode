#ifndef __SWIOT_IMPL_H__
#define __SWIOT_IMPL_H__

#include "swiot_config.h"


/**
 * ??¨¬¡§?¨¤1?
 **/
#include <string.h>
#include <stdio.h>
#include "swiot_platform.h"
#define SWDEBUG(format,...)            printf(format, ##__VA_ARGS__)
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#define DEVICE_FIND_UDPPORT        4001
#define DEVICE_FIND_TCPPORT        4002
#define DEVICE_PRODUCT_KEY      "nLdFblXy"

#define HTTP_OK                         200
#define HTTP_BAD_REQUEST                400
#define HTTP_INTERNEL_SERVER_ERROR      500
#define HTTP_UNAUTHORIZED               401
#define HTTP_NOT_FOUND                  404


#ifndef LOG_LEVEL
#define LOG_LEVEL       LOG_ERROR
#endif


#define log_error(format,...)           do {  \
                                            if(LOG_LEVEL <= LOG_ERROR) {  \
                                                SWDEBUG("[E][%s %s %d] "format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);  \
                                            }  \
                                        } while(0);
#define log_warn(format, ...)           do {  \
                                            if(LOG_LEVEL <= LOG_WARN) {  \
                                                SWDEBUG("[W][%s %s %d] "format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);  \
                                            }  \
                                        } while(0);

#define log_info(format, ...)           do {  \
                                            if(LOG_LEVEL <= LOG_INFO) {  \
                                                SWDEBUG("[I][%s %s %d] "format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);  \
                                            }  \
                                        } while(0);

#define log_debug(format, ...)          do {  \
                                            if(LOG_LEVEL <= LOG_DEBUG) {  \
                                                SWDEBUG("[D][%s %s %d] "format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);  \
                                            }  \
                                        } while(0);


#define POINTER_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            log_error("Invalid argument, %s = %p\n", #ptr, ptr); \
            return (err); \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            log_error("Invalid argument, %s = %p\n", #ptr, (ptr)); \
            return (err); \
        } \
        if (0 == strlen((ptr))) { \
            log_error("Invalid argument, %s = '%s'\n", #ptr, (ptr)); \
            return (err); \
        } \
    } while(0)

#define ZERO_SANITY_CHECK(num, err) \
    do { \
        if (0 > (num)) { \
            log_error("Invalid argument, %s = %d\n", #num, num); \
            return (err); \
        } \
    } while(0)


int url_get_httpresp(int httpstatus, int sessionid, int content_length, char *respbuf, int bufsize);

unsigned int SWIOT_Common_Get_Ms();

unsigned int SWIOT_Common_Utc();

/*int SWIOT_Common_Calc_Sign(const char* prodect_key,
        const char* device_name,
        const char* device_secret,
        char* hmac_sigbuf,
        const int hmac_buflen,
        swiot_sign_method_types_t sign_method,
        const char *timestamp_str);*/

char* SWIOT_Common_Getval( char *buf, int buflen, char *name, char *value, int valuelen );

char* SWIOT_Common_Param( char *url, char *name, char *value, int valuesize );
#endif //__SWIOT_IMPL_H__
