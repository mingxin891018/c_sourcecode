#ifndef __SWSHA1_H__
#define __SWSHA1_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 接口实现在secure分支中有 */
typedef struct sha1_context 
{
	uint32_t H[5];
	unsigned long long Length;
	int count;
	unsigned char buf[64];
}sha1_context_t;

void sw_sha1_init(sha1_context_t *context);
void sw_sha1_update(sha1_context_t *context, const unsigned char *buf, unsigned len);
int sw_sha1_final(sha1_context_t *context, unsigned char digest[20]);

#ifdef __cplusplus
}
#endif

#endif /*end __SWSHA1_H__ */