/** 
 * @file cwmp_authentication.c
 * @brief Http authentication
 * @author niujiuru
 * @date 2007-8-16
 */

// 详细请参考《RFC2617》协议标准

#include "swhttpauth.h"
#include "string.h"
#include "swmd5.h"
#include "swcommon_priv.h"

void cvt_hex(IN ahash bin, OUT ahash_hex hx_hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (bin[i] >> 4) & 0xf;
        if (j <= 9)
            hx_hex[i*2] = (j + '0');
        else
            hx_hex[i*2] = (j + 'a' - 10);
        j = bin[i] & 0xf;
        if (j <= 9)
            hx_hex[i*2+1] = (j + '0');
        else
            hx_hex[i*2+1] = (j + 'a' - 10);
    };
    hx_hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void digest_calc_ha1(
        IN char * psz_alg,	/* 算法名称 md5||md5-sess */
        IN char * psz_username,	/* login user name */
        IN char * psz_realm,	/* realm name */
        IN char * psz_password,	/* login password */
        IN char * psz_nonce,	/* 服务器随机产生的nonce返回串 */
        IN char * psz_c_nonce,	/* 客户端随机产生的nonce串 */
        OUT ahash_hex hx_session_key  /* H(A1) */
        )
{
    md5_context_t md5_ctx;
    ahash ha1;

    sw_md5_init(&md5_ctx);
    sw_md5_update(&md5_ctx, (unsigned char*)psz_username, strlen(psz_username));
    sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
    sw_md5_update(&md5_ctx, (unsigned char*)psz_realm, strlen(psz_realm));
    sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
    sw_md5_update(&md5_ctx, (unsigned char*)psz_password, strlen(psz_password));
    sw_md5_final((unsigned char*)ha1, &md5_ctx);
#ifdef WIN32
    if (stricmp(psz_alg, "md5-sess") == 0) {
#else
        if (strcasecmp(psz_alg, "md5-sess") == 0) {
#endif
            sw_md5_init(&md5_ctx);
            sw_md5_update(&md5_ctx, (unsigned char*)ha1, HASHLEN);
            sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
            sw_md5_update(&md5_ctx, (unsigned char*)psz_nonce, strlen(psz_nonce));
            sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
            sw_md5_update(&md5_ctx, (unsigned char*)psz_c_nonce, strlen(psz_c_nonce));
            sw_md5_final((unsigned char*)ha1, &md5_ctx);
        };
        cvt_hex(ha1, hx_session_key);
    }

    /* calculate request-digest/response-digest as per HTTP Digest spec */
    void digest_calc_respose(
            IN ahash_hex ha1,           /* H(A1) */
            IN char * psz_nonce,       /* nonce from server */
            IN char * psz_nonce_count,  /* 8 hex digits */
            IN char * psz_c_nonce,      /* client nonce */
            IN char * psz_qop,         /* qop-value: "", "auth", "auth-int" */
            IN char * psz_method,      /* method from the request */
            IN char * psz_digest_uri,   /* requested URL */
            IN ahash_hex hx_entity,       /* H(entity body) if qop="auth-int" */
            OUT ahash_hex hx_response      /* request-digest or response-digest */
            )
    {
        md5_context_t md5_ctx;
        ahash ha2;
        ahash resp_hash;
        ahash_hex ha2_hex;

        // calculate H(A2)
        sw_md5_init(&md5_ctx);
        sw_md5_update(&md5_ctx, (unsigned char*)psz_method, strlen(psz_method));
        sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
        sw_md5_update(&md5_ctx, (unsigned char*)psz_digest_uri, strlen(psz_digest_uri));
#ifdef WIN32
        if (stricmp(psz_qop, "auth-int") == 0) {
#else
            if (strcasecmp(psz_qop, "auth-int") == 0) {
#endif
                sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
                sw_md5_update(&md5_ctx, (unsigned char*)hx_entity, HASHHEXLEN);
            };
            sw_md5_final((unsigned char*)ha2, &md5_ctx);
            cvt_hex(ha2, ha2_hex);

            // calculate response
            sw_md5_init(&md5_ctx);
            sw_md5_update(&md5_ctx, (unsigned char*)ha1, HASHHEXLEN);
            sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
            sw_md5_update(&md5_ctx, (unsigned char*)psz_nonce, strlen(psz_nonce));
            sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
            if (*psz_qop) {
                sw_md5_update(&md5_ctx, (unsigned char*)psz_nonce_count, strlen(psz_nonce_count));
                sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
                sw_md5_update(&md5_ctx, (unsigned char*)psz_c_nonce, strlen(psz_c_nonce));
                sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
                sw_md5_update(&md5_ctx, (unsigned char*)psz_qop, strlen(psz_qop));
                sw_md5_update(&md5_ctx, (unsigned char*)":", 1);
            };
            sw_md5_update(&md5_ctx, (unsigned char*)ha2_hex, HASHHEXLEN);
            sw_md5_final((unsigned char*)resp_hash, &md5_ctx);
            cvt_hex(resp_hash, hx_response);
        }
