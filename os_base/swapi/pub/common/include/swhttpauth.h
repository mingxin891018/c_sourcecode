/** 
 * @file cwmp_authentication.c
 * @brief Http authentication
 * @author niujiuru
 * @date 2007-8-16
 */

// 详细请参考《RFC2617》协议标准

#ifndef __HTTPAUTHENTICATIONRFC2617_H__
#define __HTTPAUTHENTICATIONRFC2617_H__

#define HASHLEN 16
typedef char ahash[HASHLEN];
#define HASHHEXLEN 32
typedef char ahash_hex[HASHHEXLEN + 1];
#define IN
#define OUT

#ifdef __cplusplus
extern "C"
{
#endif
/* calculate H(A1) as per HTTP Digest spec */
void digest_calc_ha1(
		    IN char * psz_alg,	            /* 算法名称 md5||md5-sess */
		    IN char * psz_username,	        /* login user name */
		    IN char * psz_realm,	        /* realm name */
		    IN char * psz_password,	        /* login password */
		    IN char * psz_nonce,	        /* 服务器随机产生的nonce返回串 */
		    IN char * psz_c_nonce,	        /* 客户端随机产生的nonce串 */
		    OUT ahash_hex hx_session_key    /* H(A1) */
		    );
/* calculate request-digest/response-digest as per HTTP Digest spec */
void digest_calc_respose(
			IN ahash_hex ha1,               /* H(A1) */
			IN char * psz_nonce,            /* nonce from server */
			IN char * psz_nonce_count,      /* 8 hex digits */
			IN char * psz_c_nonce,          /* client nonce */
			IN char * psz_qop,              /* qop-value: "", "auth", "auth-int" */
			IN char * psz_method,           /* method from the request */
			IN char * psz_digest_uri,       /* requested URL */
			IN ahash_hex hx_entity,         /* H(entity body) if qop="auth-int" */
			OUT ahash_hex hx_response       /* request-digest or response-digest */
			);
#ifdef __cplusplus
}
#endif

#endif
