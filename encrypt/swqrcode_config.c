#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swapi.h"
#include "swapp_priv.h"

#define CHECK_PARAM(handle)    \
		do  \
		{   \
			if(!handle) \
				return NULL;    \
		}while(0)

#define INFO_MAX 512

typedef struct sw_qrcode_info_ {
	char K1[INFO_MAX];
	int K1_len;

	char K2[INFO_MAX];
	int K2_len;

	char *E2;
	int E2_len;

	char *S2;
	char *ssid;
	char *passwd;
	char *sec;
}sw_qrcode_info_t;

static const char* g_szV64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char Base64Val(wchar_t chr)
{
	unsigned char i;

	for(i=0; i<64; i++)
	{
		if(g_szV64[i] == chr)
			return i;
	}

	if(chr == '=')
		return 64;

	return (unsigned char)255;
}

static int DecodeBase64T(const void* pBase64Data, unsigned int nCount, int bIsWideChar, unsigned char** ppRet)
{
	int nSize;
	int nIndex = 0;
	unsigned int i, loop;
	unsigned char val;
	wchar_t item;
	const char* pAnsiData = 0;
	const wchar_t* pUnicodeData = 0;

	if(bIsWideChar)
		pUnicodeData = (const wchar_t*)pBase64Data;
	else
		pAnsiData = (const char*)pBase64Data;

	if(nCount == 0)
	{
		if(pUnicodeData)
			while(pUnicodeData[nCount++] != '\0');
		else
			while(pAnsiData[nCount++] != '\0');

		nCount--;
	}

	nSize = nCount * 6/8 + 1;

	*ppRet = (unsigned char*)malloc(nSize * sizeof(unsigned char));
	memset(*ppRet, 0, nSize);

	for(i = 0; i < nCount; i+=4)
	{
		for(loop=0; loop<4; loop++)
		{
			if(pUnicodeData)
				item = pUnicodeData[i + loop];
			else
				item = pAnsiData[i + loop];

			if( (i + loop >= nCount) || (item == '=') )
			{
				return nIndex;
			}

			val = Base64Val(item);
			if(val >= 64)
			{
				free(*ppRet);
				*ppRet = 0;
				return 0;
			}

			if(loop == 0)
			{
				(*ppRet)[nIndex] = val<<2;	//XXXX XX|xx, xxxx | xxxx, xx|xx xxxx
			}
			else if(loop == 1)
			{
				(*ppRet)[nIndex] |= (val >> 4);			//XXXX XX|XX, xxxx | xxxx, xx|xx xxxx
				(*ppRet)[++nIndex] = (val & 0xF) << 4;	//xxxx xx|xx, XXXX | xxxx, xx|xx xxxx
			}
			else if(loop == 2)
			{
				(*ppRet)[nIndex] |= (val >> 2);				//xxxx xx|xx, XXXX | XXXX, xx|xx xxxx
				(*ppRet)[++nIndex] = ((val & 0x3) << 6);	//xxxx xx|xx, xxxx | xxxx, XX|xx xxxx
			}
			else
			{
				(*ppRet)[nIndex++] |= val;	//xxxx xx|xx, xxxx | xxxx, XX|XX XXXX
			}
		}		
	}

	return nIndex;
}

/**
 * 加密二维码字符串
 * @param key 设备码
 * @param text 明文
 * @param result 密文
 * @param length 明文长度
 * */
static void encryptWIFIInfo(const char *key,const char *text,char *result,size_t length){
	unsigned long keylen,textlen;
	int index = 0, i = 0;
	keylen = strlen(key);
	textlen = length;
	int keyArr[keylen * 2];
	int reversedKeyArr[keylen * 2];
	for (i = 0; i < keylen; i++) {
		keyArr[2*i] = key[i]/10;
		keyArr[2*i + 1] = key[i]%10;
		reversedKeyArr[keylen*2 - 2 - 2*i] = key[i]/10;
		reversedKeyArr[keylen*2 - 1 - 2*i] = key[i]%10;
	}
	if (result) {
		for (i=0; i < textlen; i++) {
			if (i%2 == 0) {
				index = (i%(keylen * 4))/2;
				result[i] = text[i]^keyArr[index];
			} else {
				index = (i%(keylen * 4) - 1)/2;
				result[i] = text[i]^reversedKeyArr[index];
			}
		}
	}
}

/*	加密方式：
 *	二维码明文信息：SSID:EZVIZ-AB1-AQ7V;SEC:3;P:12345678;IP:192.168.1.10;MASK:255.255.255.0;GW:192.168.1.1;DNS:192.168.1.1;C:D1C4AE6E3D448786
 *	
 *	S1=SSID=EZVIZ-AB1-AQ7V
 *	S2= wifi密码;sec =  P:12345678;C:D1C4AE
 *	
 *	K1=base64(S1)
 *	E2=base64(异或加密(S2))
 *
 *	二维码密文信息：K3=K1|K2 = RVpWSVotQUIxLUFRN1Y=|VT0wNzUyMD4wPjNKPEM3SzJEQw==
 *
 */
sw_qrcode_info_t *sw_swqrcode_analysis(unsigned char *qrcode, unsigned char *key)
{
	CHECK_PARAM(qrcode);
	
	sw_qrcode_info_t *info = NULL;
	unsigned char *p = strstr(qrcode, "|");
	if(p == NULL)
		return NULL;
	
	int k1_len = p - qrcode ;
	int k2_len = strlen(p + 1);
	
	if(k1_len >= INFO_MAX || k2_len >= INFO_MAX){
		SW_APP_LOG_ERROR("info too lang!\n");
		return NULL;
	}
	
	info = malloc(sizeof(sw_qrcode_info_t));
	if(info == NULL){
		SW_APP_LOG_ERROR("malloc failed!\n");
		goto analysis_error;
	}
	memset(info, 0, sizeof(sw_qrcode_info_t));

	info->K1_len = k1_len;
	info->K2_len = k2_len;
	memcpy(info->K1, qrcode, k1_len);
	memcpy(info->K2, p + 1, k2_len);
	SW_APP_LOG_DEBUG("K1=%s\nK2=%s\n", info->K1, info->K2);

	DecodeBase64T(info->K1, strlen(info->K1), 0, &p);
	info->ssid = p;
	
	info->E2_len = DecodeBase64T(info->K2, strlen(info->K2), 0, &p);
	info->E2 = p;
	
	p = malloc(info->E2_len);
	if(p == NULL){
		SW_APP_LOG_ERROR("malloc failed!\n");
		goto analysis_error;
	}
	encryptWIFIInfo(key, info->E2, p, info->E2_len);
	info->S2 = p;
	
	unsigned char *q = strstr(p, "P:");
	info->passwd = (q + 2);

	unsigned char *r = strstr(p, ";");
	if(r == NULL){
		SW_APP_LOG_ERROR("analysis error!\n");
		goto analysis_error;
	}
	*r = '\0';
	
	q = strstr(r + 1, "C:");
	if(q == NULL){
		SW_APP_LOG_ERROR("analysis error!\n");
		goto analysis_error;
	}

	info->sec = (q + 2);
	SW_APP_LOG_DEBUG("SSID=%s\n", info->ssid);
	SW_APP_LOG_DEBUG("P=%s\n", info->passwd);
	SW_APP_LOG_DEBUG("C=%s\n", info->sec);

	return info;

analysis_error:
	if(info){
		if(info->S2)
			free(info->S2);
		if(info->E2)
			free(info->E2);
		if(info->ssid)
			free(info->ssid);

		free(info);
	}
	return NULL;
}

char *sw_qrcode_get_ssid(sw_qrcode_info_t *info)
{
	CHECK_PARAM(info);
	
	return info->ssid;
}


char *sw_qrcode_get_passwd(sw_qrcode_info_t *info)
{
	CHECK_PARAM(info);

	return info->passwd;
}

char *sw_qrcode_get_sec(sw_qrcode_info_t *info)
{
	CHECK_PARAM(info);

	return info->sec;
}

void sw_swqrcode_free(sw_qrcode_info_t *info)
{
	if(info){
		if(info->S2)
			free(info->S2);
		if(info->E2)
			free(info->E2);
		if(info->ssid)
			free(info->ssid);

		free(info);
	}
}
/*
void main(void)
{
	sw_swqrcode_analysis("RVpWSVotQUIxLUFRN1Y=|VT0wNzUyMD4wPjNKPEM3SzJEQw==", "3ANBB231722NEDK");

}
*/
