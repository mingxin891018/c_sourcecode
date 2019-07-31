/*
 * Generic hashmap manipulation functions
 */
#ifndef __SWHASHMAP_H__
#define __SWHASHMAP_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	KEY_DIGITAL=1,
	KEY_STRING
}key_type_t;

#define MAP_INVALID_KEY -4  /* Key is NULL */
#define MAP_MISSING -3      /* No such element */
#define MAP_FULL -2         /* Hashmap is full */
#define MAP_OMEM -1         /* Out of Memory */
#define MAP_OK 0            /* OK */

/* We need to keep keys and values */
typedef struct hashmap_element
{
	void* key;
	int     in_use;	/* 为0时空闲，1使用中，-1删除 */
	uint32_t hash;
	void* value;
	
}hashmap_element_t;

/* A hashmap has some maximum size and current size,
 *  *  as well as the data to hold.
 *   */
struct swhashmap
{
	int table_size;
	int size;
	hashmap_element_t *data;
	int key_type;
	HANDLE lock;
};


typedef struct swhashmap swhashmap_t;
/*
 * Return an empty hashmap. Returns NULL if empty.
 */
swhashmap_t* sw_hashmap_create(int size,key_type_t key_type);

/*
 * 销毁一个hashmap
 */
void sw_hashmap_destroy(swhashmap_t* map);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
int sw_hashmap_put(swhashmap_t* map, void* key, void* value);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int sw_hashmap_get(swhashmap_t* map, void* key,  void** value);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int sw_hashmap_remove(swhashmap_t* map, void* key);

/*
 * Get the current size of a hashmap
 */
int sw_hashmap_size(swhashmap_t* map);

/*
 * Get your pointer out of the hashmap by index
 */
int sw_hashmap_get_byindex(swhashmap_t* m, int index,void** key, void** value);

#ifdef __cplusplus
}
#endif

#endif /*__SWHASHMAP_H__*/
