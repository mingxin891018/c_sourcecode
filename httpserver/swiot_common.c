#include "swiot_common.h"
//#include "utils_hmac.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"

int url_get_httpresp(int httpstatus, int sessionid, int content_length, char *respbuf, int bufsize)
{
    int resp_len = 0;

    switch( httpstatus )
    {
    case HTTP_OK:
        resp_len = snprintf(respbuf, bufsize,
                            "HTTP/1.1 200 OK\r\n"
                            "SessionId: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-Length: %d\r\n"
                            "Content-Type: text/json;charset=UTF-8\r\n\r\n"
                            , sessionid, content_length );
        break;
    case HTTP_BAD_REQUEST:
        resp_len = snprintf(respbuf, bufsize,
                            "HTTP/1.1 400 Bad Request\r\n"
                            "SessionId: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-Type: text/json;charset=UTF-8\r\n"
                            "Content-Length: %d\r\n\r\n", sessionid, content_length );
        break;
    case HTTP_INTERNEL_SERVER_ERROR:
        resp_len = snprintf(respbuf, bufsize,
                            "HTTP/1.1 500 Internal Server Error\r\n"
                            "SessionId: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-Type: text/json;charset=UTF-8\r\n"
                            "Content-Length: %d\r\n\r\n", sessionid, content_length );
        break;
    case HTTP_UNAUTHORIZED:
        resp_len = snprintf(respbuf, bufsize,
                            "HTTP/1.1 401 Unauthorized\r\n"
                            "SessionId: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-Type: text/json;charset=UTF-8\r\n"
                            "Content-Length: %d\r\n\r\n", sessionid, content_length );
        break;
    case HTTP_NOT_FOUND:
        resp_len = snprintf(respbuf, bufsize,
                            "HTTP/1.1 404 Not Found\r\n"
                            "SessionId: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-Type: text/json;charset=UTF-8\r\n"
                            "Content-Length: %d\r\n\r\n", sessionid, content_length );
        break;
    default:
        break;
    }

    return resp_len;
}

unsigned int SWIOT_Common_Get_Ms()
{
    static int first = 1;
    static struct timeval start;
    struct timeval now;

    gettimeofday( &now, NULL );

    if( first )
    {
        start = now;
        first = 0;
        return 0;
    }
    else
    {
        return (now.tv_sec-start.tv_sec)*1000 + now.tv_usec/1000 - start.tv_usec/1000;
    }
}

unsigned int SWIOT_Common_Utc()
{
    return (uint32_t)time(NULL);
}

/*
int SWIOT_Common_Calc_Sign(const char* product_key,
        const char* device_name,
        const char* device_secret,
        char* hmac_sigbuf,
        const int hmac_buflen,
        swiot_sign_method_types_t sign_method,
        const char *timestamp_str)
{
    char signature[64];
    char hmac_source[256];

    STRING_PTR_SANITY_CHECK(product_key, -1);
    STRING_PTR_SANITY_CHECK(device_name, -1);
    STRING_PTR_SANITY_CHECK(device_secret, -1);
    POINTER_SANITY_CHECK(hmac_sigbuf, -1);
    POINTER_SANITY_CHECK(timestamp_str, -1);

    memset(signature, 0, sizeof(signature));
    memset(hmac_source, 0, sizeof(hmac_source));
    snprintf(hmac_source,
                      sizeof(hmac_source),
                      "deviceName%s_productKey%s_timestamp%s"   ,
                       device_name,
                       product_key,
                       timestamp_str );

    if (sign_method == SWIOT_SIGN_METHOD_TYPE_SHA) {
        utils_hmac_sha1(hmac_source, strlen(hmac_source),
                    signature,
                    device_secret,
                    strlen(device_secret));
    } else if (sign_method == SWIOT_SIGN_METHOD_TYPE_MD5) {
        utils_hmac_md5(hmac_source, strlen(hmac_source),
                   signature,
                   device_secret,
                   strlen(device_secret));
    }

    memcpy(hmac_sigbuf, signature, hmac_buflen);
    return 0;
}
*/

char* SWIOT_Common_Getval( char *buf, int buflen, char *name, char *value, int valuelen )
{
    char *s, *e, *p;
    size_t len;
    size_t size;

    len = buflen >= 0 ? buflen : strlen(buf);

    memset( value, 0, valuelen );

    p = s = buf;
    while( p < buf+len )
    {
        /* 去掉开始的空格 */
        while( *p == ' ' ||  *p == ',' ||  *p == '\t' || *p == '\r' || *p == '\n' )
            p++;

        /* 记录行开始 */
        s = p;

        /* 找到行结束 */
        while( *p != '\r' && *p != '\n' && *p != '\0' )
            p++;
        e = p;

        /* 找到需要的字段 */
        if( memcmp( s, name, (int)strlen(name) ) == 0 )
        {
            /* 找到名称结束 */
            p = s;
            while( *p != ':' &&  *p != '=' &&  *p != '\t' && *p != '\r' && *p != '\n' && *p != '\0' )
                p++;
            if( *p == 0 || e <= p )
                goto NEXT_LINE;

            /* 找到值开始 */
            p++;
            while( *p == ' ' || *p == ':' ||  *p == '=' ||
                    *p == ',' || *p == '\t' || *p == '\r' || *p == '\n' )
            {
                p++;
            }
            while( s < e && *e == ' ' )
                e--;
            size = (valuelen-1)<(e-p) ? (valuelen-1):(e-p);
            if( value && 1 < valuelen )
                strncpy( value, p, (int)size );
            return value;
        }
NEXT_LINE:
        p = e + 1;
    }
    return NULL;
}


char* SWIOT_Common_Param( char *url, char *name, char *value, int valuesize )
{
    int i=0;
    char *p;

    p = strstr( url, name );

    while( p != NULL )
    {
        if( ( p == url  || ( p > url && ( *(p-1) == '?' || *(p-1) == ';' || *(p-1) == '&' || *(p-1) == ',') ) )
                &&  *(p+strlen(name)) == '=' )
        {
            p = p + strlen(name);
            p++;

            i=0;
            for(;; p++)
            {
                if( *p==';' || *p=='&' || *p==',' || *p=='\0'|| *p==' ' )
                {
                    break;
                }
                if( i < valuesize-1 )
                {
                    value[i] = *p;
                    i++;
                }
                else
                    break;
            }
            value[i] = '\0';
            return value;
        }
        else
            p = p + strlen(name);

        p = strstr(p,name);
    }

    return NULL;
}

