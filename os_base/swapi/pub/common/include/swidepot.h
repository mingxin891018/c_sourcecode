/**
 * @file swidepot.h
 * @brief 定义参数仓库接口
 * @author chenkai
 * @history chenkai 2011-2-23 created
 */
#ifndef __SWIDEPOT_H__
#define __SWIDEPOT_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum pa_attribute
{
	PA_MACADDR = 0x02,			/* mac地址 */
	PA_IPADDR = 0x4,			/* ip地址 */
	PA_INT = 0x8,				/* 整数 */
	PA_STRING= 0x10,			/* 串 */
	PA_NETMODE= 0x20,			/* 串 */
	PA_BROWSERREGION= 0x80,		/* browser regioin */
}pa_attribute_t;

//定义e2prom 参数列表
typedef struct _e2_map
{
    char* name;
    int attribute;
    uint32_t offset;
    uint32_t size;
}e2_map_t;

//定义仓库的类型
typedef enum
{
	IDEPOT_E2PROM = 0,
    IDEPOT_JAVA,
	IDEPOT_FLASH,
	IDEPOT_FILE,
}idepot_type_t;

typedef struct _sw_idepot sw_idepot_t;

//定义参数展开的回调函数,返回参数的位置
typedef int (*spread_para_func)(sw_idepot_t *depot,char* name,char* value);

//定义参数收集的回调函数,返回-1表示没有后续的参数
typedef int (*gather_para_func)(sw_idepot_t *depot,int pos,char** name,char** value);

//定义参数仓库结构
struct _sw_idepot
{
	char  name[64];
	int   type;
	bool  modified;
	bool  autosave;
    bool  ( *load)( sw_idepot_t *depot,spread_para_func spread);
	bool  ( *get )( sw_idepot_t *depot, char* name ,char* value,int size );
	bool  ( *set )( sw_idepot_t *depot, char* name ,char* value );
	bool  ( *save)(sw_idepot_t *depot, gather_para_func gather );
};

/**
 * @brief 打开e2prom 参数仓库
 *
 * @param map e2prom 参数列表
 * @param size map的长度
 *
 * @return 返回创建的idepot对象
 */
sw_idepot_t*  sw_e2promdepot_open(e2_map_t* map,int size,uint32_t devaddr);

/**
 * @brief 关闭创建的idepot对象
 *
 * @param idepot对象
 */
void  sw_e2promdepot_close(sw_idepot_t* depot);


/**
 * @brief 打开flash 参数仓库
 *
 * @param addr 存放参数的flash地址
 * @param pt_size 分区大小
 * @param buf_size 处理参数的buf大小，推荐值为128*1024
 *
 * @return 返回创建的idepot对象
 */
sw_idepot_t* sw_iflashdepot_open(uint64_t addr,int pt_size,int buf_size);

/**
 * @brief 关闭创建的idepot对象
 *
 * @param idepot对象
 */
void sw_iflashdepot_close(sw_idepot_t* depot);

/**
 * @brief 设置flash depot备份分区的地址
 *
 * @param idepot对象
 * @param addr,备份分区的地址
 *
 * @return 成功返回0，否则返回-1
 */
int sw_iflashdepot_set_backup(sw_idepot_t* depot,uint64_t addr);

/**
 * @brief 设置参数分区分区地址
 *
 * @param addr 参数分区的地址
 */
int sw_iflashdepot_set_addr(sw_idepot_t* depot, uint64_t addr);

/**
 * @brief 设置参数分区分区大小
 *
 * @param size 参数分区的大小
 */
int sw_iflashdepot_set_size(sw_idepot_t* depot, int size);

/**
 * @brief 打开文件参数仓库
 *
 * @param filename 文件路径
 *
 * @return 返回创建的idepot对象
 */
sw_idepot_t* sw_ifiledepot_open(char* filename);


/**
 * @brief 关闭创建的idepot对象
 *
 * @param idepot对象
 */
void sw_ifiledepot_close(sw_idepot_t* depot);


#ifdef __cplusplus
}
#endif

#endif //__SWIDEPOT_H__
	
