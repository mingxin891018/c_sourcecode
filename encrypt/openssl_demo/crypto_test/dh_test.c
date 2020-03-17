#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

static int cb(int p, int n, BN_GENCB *arg);

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X ", data[i]&0xff);
	printf("\n");
}

DH *dh_init(int g)
{
	int i = 0;
	DH *dh = NULL;
	BN_GENCB *_cb = NULL;

	//设置回调
	_cb = BN_GENCB_new();
	if(!_cb)
	{
		log_error("malloc _cb failed!\n");
		goto ERR_END;
	}
	BN_GENCB_set(_cb, &cb, NULL);

	//初始化dh结构体
	dh = DH_new();
	if(!dh)
	{
		log_error("DH_new failed!\n");
		goto ERR_END;
	}

	//生成公钥私钥
	switch(g)
	{
		case 5:
			if(DH_generate_parameters_ex(dh, 128, DH_GENERATOR_5, _cb) != 1)//128bit
			{
				log_error("DH_generate_parameters_ex failed!\n");
				goto ERR_END;
			}
			break;
		case 2:
			if(DH_generate_parameters_ex(dh, 128, DH_GENERATOR_2, _cb) != 1)
			{
				log_error("DH_generate_parameters_ex failed!\n");
				goto ERR_END;
			}
			break;
		default:
			log_error("dh_g param error!\n");
			break;
	}
	if (!DH_check(dh, &i))
	{
		log_error("DH_check dh %d failed!\n", i);
		goto ERR_END;
	}
    if (!DH_generate_key(dh))
	{
		log_error("DH_generate_key failed!\n");
        goto ERR_END;
	}
	log_info("get dh_key success!\n");
	return dh;

ERR_END:
	if(dh)
		DH_free(dh);
	dh = NULL;
	return dh;
}

DH *dh_init_ex(const unsigned char *dh_p, int dh_p_len, const unsigned char *dh_g, int dh_g_len)
{
	int i = 0;
	DH *dh = NULL;
	BIGNUM *bn_dh_p = NULL, *bn_dh_g = NULL;

	//初始化dh结构体
	dh = DH_new();
	if(!dh)
	{
		log_error("DH_new failed!\n");
		goto ERR_END;
	}

	bn_dh_p = BN_bin2bn(dh_p, dh_p_len, NULL);
	bn_dh_g = BN_bin2bn(dh_g, dh_g_len, NULL);
	if(!bn_dh_p || !bn_dh_g)
	{
		log_error("new bignum failed!\n");
		goto ERR_END;
	}

	if(DH_set0_pqg(dh, bn_dh_p, NULL, bn_dh_g) != 1)
	{
		log_error("DH_set0_pqg failed!\n");
		goto ERR_END;
	}

	if (!DH_check(dh, &i))
	{
		log_error("DH_check dh %d failed!\n", i);
		goto ERR_END;
	}
	if (!DH_generate_key(dh))
	{
		log_error("DH_generate_key failed!\n");
		goto ERR_END;
	}
	log_info("get dh_key success!\n");
	return dh;

ERR_END:
	if(dh)
		DH_free(dh);
	return NULL;
}

int dh_get_pg(DH *dh, unsigned char **dh_p, int *dh_p_len, unsigned char **dh_g, int *dh_g_len)
{
	const BIGNUM *bn_dh_p = NULL, *bn_dh_g = NULL;
	unsigned char *tmp_dh_p = NULL, *tmp_dh_g = NULL;
	int ret = 0, tmp_dh_p_length = 0, tmp_dh_g_length = 0;
	if(dh == NULL)
	{
		log_error("param error!\n");
		goto ERR_END;
	}

	//获取dh_p，dh_g大整数值
	DH_get0_pqg(dh, &bn_dh_p, NULL, &bn_dh_g);
	if(!bn_dh_p || !bn_dh_g)
	{
		log_error("DH_get0_pqg failed!\n");
		goto ERR_END;
	}

	//转换dh_p大整数值为二进制值
	tmp_dh_p_length = BN_num_bytes(bn_dh_p);
	tmp_dh_p = (unsigned char *)malloc(tmp_dh_p_length);
	if(!tmp_dh_p)
	{
		log_error("malloc pubkey failed!\n");
		goto ERR_END;
	}
	memset(tmp_dh_p, 0, tmp_dh_p_length);
	//BN_bn2bin转换后的值是把高位0去掉的， 输出的字节长度不一定就是二进制数据的字节数，一定不能忽视。
	ret = BN_bn2bin(bn_dh_p, tmp_dh_p);
	log_info("dh_p_len=%d, bn_dh_p_len=%d!\n", tmp_dh_p_length, ret);
	*dh_p = tmp_dh_p;
	*dh_p_len = tmp_dh_p_length;

	//转换dh_g大整数值为二进制值
	tmp_dh_g_length = BN_num_bytes(bn_dh_g);
	tmp_dh_g = (unsigned char *)malloc(tmp_dh_g_length);
	if(!tmp_dh_g)
	{
		log_error("malloc pubkey failed!\n");
		goto ERR_END;
	}
	memset(tmp_dh_g, 0, tmp_dh_g_length);
	//BN_bn2bin转换后的值是把高位0去掉的， 输出的字节长度不一定就是二进制数据的字节数，一定不能忽视。
	ret = BN_bn2bin(bn_dh_g, tmp_dh_g);
	log_info("dh_g_len=%d, bn_dh_g_len=%d,g=%d!\n", tmp_dh_g_length, ret, tmp_dh_g[0]);
	*dh_g = tmp_dh_g;
	*dh_g_len = tmp_dh_g_length;
	return ret;

ERR_END:
	if(tmp_dh_p)
		free(tmp_dh_p);
	if(tmp_dh_g)
		free(tmp_dh_g);
	return -1;

}

int dh_set_pg(DH *dh, const unsigned char *dh_p, int dh_p_len, const unsigned char *dh_g, int dh_g_len)
{
	int ret = -1;
	BIGNUM *bn_dh_p = NULL, *bn_dh_g = NULL;

	if(dh == NULL)
	{
		log_error("param error!\n");
		goto ERR_END;
	}
	bn_dh_p = BN_new();
	bn_dh_g = BN_new();
	if(!bn_dh_p || bn_dh_g)
	{
		log_error("new bignum failed!\n");
		goto ERR_END;
	}
	BN_bin2bn(dh_p, dh_p_len, bn_dh_p);
	BN_bin2bn(dh_g, dh_g_len, bn_dh_g);

	ret = DH_set0_pqg(dh, bn_dh_p, NULL, bn_dh_g);
	if(ret != 1)
	{
		log_error("DH_set0_pqg failed!\n");
		goto ERR_END;
	}
	ret = 0;

ERR_END:
	if(bn_dh_p)
		 BN_free(bn_dh_p);
	if(bn_dh_g)
		 BN_free(bn_dh_g);

	return ret;
}

int dh_get_pubkey(DH *dh, unsigned char **pubkey, int *pubkey_len, unsigned char **privkey, int *privkey_len)
{
	const BIGNUM *bn_pubkey = NULL, *bn_privkey = NULL;
	unsigned char *tmp_pubkey = NULL, *tmp_privkey = NULL;
	int ret = 0, tmp_pubkey_length = 0, tmp_privkey_length = 0;
	if(dh == NULL)
	{
		log_error("param error!\n");
		goto ERR_END;
	}
	//获取大整数pubkey和privkey
	DH_get0_key(dh, &bn_pubkey, &bn_privkey);
	if(!bn_pubkey || !bn_privkey)
	{
		log_error("DH_get0_key failed!\n");
		goto ERR_END;
	}

	//大整数bn_pubkey转换为二进制数pubkey
	if(pubkey != NULL && pubkey_len != NULL)
	{
		tmp_pubkey_length = BN_num_bytes(bn_pubkey);
		tmp_pubkey = (unsigned char *)malloc(tmp_pubkey_length);
		if(!tmp_pubkey)
		{
			log_error("malloc pubkey failed!\n");
			goto ERR_END;
		}
		memset(tmp_pubkey, 0, tmp_pubkey_length);
		//BN_bn2bin转换后的值是把高位0去掉的， 输出的字节长度不一定就是20个字节， 一定不能忽视。
		ret = BN_bn2bin(bn_pubkey, tmp_pubkey);
		log_info("bn_pubkey length=%d, pubkey length=%d!\n", tmp_pubkey_length, ret);
		*pubkey = tmp_pubkey;
		*pubkey_len = tmp_pubkey_length;
	}

	//大整数bn_pubkey转换为二进制数pubkey
	if( privkey != NULL && privkey_len != NULL)
	{
		tmp_privkey_length = BN_num_bytes(bn_privkey);
		tmp_privkey = (unsigned char *)malloc(tmp_privkey_length);
		if(!tmp_privkey)
		{
			log_error("malloc privkey failed!\n");
			goto ERR_END;
		}
		memset(tmp_privkey, 0, tmp_privkey_length);
		ret = BN_bn2bin(bn_privkey, tmp_privkey);
		log_info("bn_privkey length=%d, privkey length=%d!\n", tmp_privkey_length, ret);
		*privkey = tmp_privkey;
		*privkey_len = tmp_privkey_length;
	}
	return 0;

ERR_END:
	if(tmp_pubkey)
		free(tmp_pubkey);
	if(tmp_privkey)
		free(tmp_privkey);
	return -1;
}

int dh_get_sharekey(DH *dh, const unsigned char *pubkey, int pubkey_len, unsigned char **sharekey, int *sharekey_len)
{
	int ret = -1;
	BIGNUM *bn_pubkey = NULL;

	if(dh == NULL)
	{
		log_error("param error!\n");
		goto ERR_END;
	}

	bn_pubkey = BN_new();
	BN_bin2bn(pubkey, pubkey_len, bn_pubkey);
	ret = DH_size(dh);
	if(ret <= 0)
	{
		log_error("get dh sharekey_len  failed!\n");
		goto ERR_END;
	}
	*sharekey = (unsigned char *)malloc(ret);
	if(!(*sharekey))
	{
		log_error("malloc sharekey failed!\n");
		goto ERR_END;
	}
	*sharekey_len = DH_compute_key(*sharekey, bn_pubkey, dh);
	if(*sharekey_len <= 0)
	{
		log_error("DH_compute_key failed!\n");
		goto ERR_END;
	}
	log_info("compute sharekey success, sharekey_len=%d,DH_size=%d\n", *sharekey_len, ret);
	return 0;

ERR_END:
	if(bn_pubkey)
		 BN_free(bn_pubkey);
	if(*sharekey)
		free(*sharekey);
	*sharekey = NULL;
	*sharekey_len = 0;
	return ret;
}

void free_dhkey(DH *dh)
{
	if(!dh)
		DH_free(dh);
}

static int cb(int p, int n, BN_GENCB *arg)
{

	return 1;
}


int main(void)
{
	int dh_p_len = 0, dh_g_len = 0;
	int pubkey_len1 = 0, pubkey_len2 = 0;
	int sharekey_len1 = 0, sharekey_len2 = 0;
	unsigned char *pubkey1 = NULL, *pubkey2 = NULL, *sharekey1 = NULL, *sharekey2 = NULL, *dh_p = NULL, *dh_g = NULL;

	//初始化并获取dh1的pubkey1,p,g,sharekey1
	DH *dh1 = dh_init(5);
	if(!dh1)
	{
		log_error("get dh1 failed!\n");
		goto END;
	}
	int ret = dh_get_pg(dh1, &dh_p, &dh_p_len, &dh_g, &dh_g_len);
	if(ret < 0)
	{
		log_error("dh_get_pubkey dh1 failed!\n");
		goto END;
	}

	ret =dh_get_pubkey(dh1, &pubkey1, &pubkey_len1, NULL, NULL);
	if(ret < 0)
	{
		log_error("dh_get_pubkey dh1 failed!\n");
		goto END;
	}

	//初始化dh2，并获取pubkey2,sharekey2
	DH *dh2 = dh_init_ex(dh_p, dh_p_len, dh_g, dh_g_len);
	if(!dh2)
	{
		log_error("get dh1 failed!\n");
		goto END;
	}

	ret =dh_get_pubkey(dh2, &pubkey2, &pubkey_len2, NULL, NULL);
	if(ret < 0)
	{
		log_error("dh_get_pubkey dh1 failed!\n");
		goto END;
	}

	//生成sharekey1,sharekey2
	dh_get_sharekey(dh1, pubkey2, pubkey_len2, &sharekey1, &sharekey_len1);
	if(sharekey1 == NULL)
	{
		log_error("dh_get_sharekey sharekey1 failed!\n");
		goto END;
	}

	dh_get_sharekey(dh2, pubkey1, pubkey_len1, &sharekey2, &sharekey_len2);
	if(sharekey2 == NULL)
	{
		log_error("dh_get_sharekey sharekey2 failed!\n");
		goto END;
	}

	//打印比较sharekey1, sharekey2 是否相同
	log_info("sharekey1:\n");
	bin_print(sharekey1, sharekey_len1);
	log_info("sharekey2:\n");
	bin_print(sharekey2, sharekey_len2);

END:
	if(dh1)
		free_dhkey(dh1);
	if(dh2)
		free_dhkey(dh2);
	if(pubkey1)
		free(pubkey1);
	if(pubkey2)
		free(pubkey2);
	if(sharekey1)
		free(sharekey1);
	if(sharekey2)
		free(sharekey2);
	if(dh_p)
		free(dh_p);
	if(dh_g)
		free(dh_g);
	return 0;
}
