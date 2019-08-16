#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define PATH "./387_0604_195948.ts"

typedef struct pat_data_{
	uint8_t data[188];						//ts包
	uint8_t payload_unit_start_indicator;	//负载单元开始标志，（packet不满188字节时需填充）
	uint8_t transport_error_indicator;		//至少有几个错误字节
	uint32_t PID;							//pid
	uint8_t crc_data[4];					//crc数据
	uint8_t *valid_data;					//crc校验的数据
	uint32_t valid_data_num;				//crc校验的数据的数量

}Pat_Data;


int MakeTable(uint32_t *crc32_table)
{
	uint32_t  i = 0, j = 0,k = 0;
	for(i = 0; i < 256; i++ ) {
		k = 0;
		for(j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1 ) {
			k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
		}

		crc32_table[i] = k;
	}
}

uint32_t Crc32Calculate(uint8_t *buffer, uint32_t size, uint32_t *crc32_table)
{
	uint32_t i = 0,crc32_reg = 0xFFFFFFFF;
	for (i = 0; i < size; i++) {
		crc32_reg = (crc32_reg << 8 ) ^ crc32_table[((crc32_reg >> 24) ^ *buffer++) & 0xFF];
	}
	return crc32_reg;
}

#if 1
Pat_Data *find_pat_data(const char *path)
{
	FILE *fp = NULL;
	uint8_t data = 0x0;
	uint8_t pat_data[188] = {0};
	int i = 0, j = 0;
	Pat_Data *pat = NULL;	
	
	if(NULL == (pat = (Pat_Data *)malloc(sizeof(Pat_Data))))
		return NULL;
	memset(pat_data,0,sizeof(pat_data));
	memset(pat,0,sizeof(Pat_Data));
	
	if(path)
		if(NULL == (fp = fopen(path,"r")))
			return NULL;
	while((fread(&data,1,1,fp)) >= 0 && data != 0x47);
	fseek(fp,-1,SEEK_CUR);
	//找pat数据
	while(188 == fread(pat->data,1,188,fp)){
		if(pat->data[0] == 0x47 && (((pat->data[1] & 0x1f) << 8) | pat->data[2]) == 0x0)
			break;
	}
	//解析PAT包信息
	pat->transport_error_indicator 			= pat->data[1] >> 7;
	pat->payload_unit_start_indicator 		= pat->data[1] >> 6 & 0x1;
	pat->PID 								= (pat->data[1] << 3) | pat->data[2];

	for(i =1; i <= 188; i ++){
		if((pat->data[188-i] == 0xff) && (pat->data[187-i]) != 0xff){
			i = 188 - i;
			break;
		}
	}
	printf("[%s:%d]valid_data_num=%d\n", __func__, __LINE__,i);
	if(pat->payload_unit_start_indicator)
		i -= 5;
	else
		i -= 4;
	pat->valid_data_num = i;
	pat->valid_data = (uint8_t *)malloc(pat->valid_data_num);
	memcpy(pat->valid_data, &pat->data[4+pat->payload_unit_start_indicator], pat->valid_data_num);
	memcpy(pat->crc_data, &pat->valid_data[pat->valid_data_num-4], 4);
	
	//打印pat相关数据
	printf("[%s:%d]PAT DATA:\n", __func__, __LINE__);
	for(i =1; i <= 188; i ++){
		printf("0x%0x, ",pat->data[i-1]);
		if(i%10 == 0)
			printf("\n");
	}
	printf("\n[%s:%d]VALID DATA:\n", __func__, __LINE__);
	for(i =1; i <= pat->valid_data_num; i ++){
		printf("0x%0x, ",pat->valid_data[i-1]);
		if(i%10 == 0)
			printf("\n");
	}
	printf("\n[%s:%d]CRC DATA:\n", __func__, __LINE__);
	for(i =1; i <= 4; i ++)
		printf("0x%0x, ",pat->crc_data[i-1]);
	printf("\n");

	fclose(fp);
	fp = NULL;
	return pat;
}

int main(void)
{
	uint32_t crc32Table[256] = {0};
	
	Pat_Data *data = NULL;
	MakeTable(crc32Table);
	data = find_pat_data(PATH);
	if(data == NULL)
		return -1;
	printf("[%s:%d]crc is %0x\n",__func__, __LINE__, Crc32Calculate(data->valid_data, data->valid_data_num - 4, crc32Table));
	printf("[%s:%d]crc is %0x\n",__func__, __LINE__, Crc32Calculate(data->valid_data, data->valid_data_num, crc32Table));
	return 0;
}
#endif

