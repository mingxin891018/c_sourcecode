#ifndef __SWJSON_H__
#define __SWJSON_H__


#define VTYPE_STR    1
#define VTYPE_OBJ    2
#define VTYPE_ARRAY  3


typedef struct sw_json_t sw_json_t;
typedef struct json_name_value_t json_name_value_t;
typedef struct json_object_t json_object_t;
typedef struct json_array_t json_array_t;

struct sw_json_t
{
	int   type;
	union
	{
		char* str;
		json_object_t* jobj;
		json_array_t* jarr;
	};
};

struct json_name_value_t
{
	char* name;
	sw_json_t value;
};


struct json_object_t
{
	json_name_value_t* nvs;
	int count;
};

struct json_array_t
{
	sw_json_t* values;
	int count;
};


#ifdef __cplusplus
extern "C"{
#endif



/***
 *   解析 json 串，结果是一个 sw_json_t 对象指针
 * @param str: json 串
 * @param length: str 长度
 * @param buf: 解析 json 串使用的缓冲区，存放解析结果
 * @param size: buf 的大小
 * @return: 解析结果对象的指针，NULL 表示解析失败
 * 注：解析结果存放在 buf 中，因此不需要单独销毁
 */
sw_json_t* sw_json_decode( const char* str, int length, void* buf, int size );


#ifdef __cplusplus
}
#endif


#endif


