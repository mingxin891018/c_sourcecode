/*
 * Generic map implementation. This class is thread-safe.
 * free() must be invoked when only one thread has access to the hashmap.
 */
 
#include "swapi.h"
#include "swhashmap.h"
#include "swmutex.h"
#include "swmem.h"
#include "swutil_priv.h"

#define hashmap_malloc(size) malloc(size) 
#define hashmap_free(buf) free(buf) 
#define hashmap_strdup(string) strdup(string)

#define MAP_INVALID_KEY -4  /* Key is NULL */
#define MAP_MISSING -3		/* No such element */
#define MAP_FULL -2			/* Hashmap is full */
#define MAP_OMEM -1			/* Out of Memory */
#define MAP_OK 0			/* OK */

static int hashmap_put(swhashmap_t* m, uint32_t hash, void* key, void* value);

static inline void lock(swhashmap_t* m)
{
	if (m->lock)
		sw_mutex_lock(m->lock);
}
static inline void unlock(swhashmap_t* m)
{
	if (m->lock)
		sw_mutex_unlock(m->lock);
}
/*
 * Return an empty hashmap. Returns NULL if empty.
 */
swhashmap_t* sw_hashmap_create(int size,key_type_t key_type)
{
	swhashmap_t* m=(swhashmap_t*) hashmap_malloc(sizeof(swhashmap_t));
	if(!m ) 
		goto err;
	if( size <0 )
		size = 1024;
	m->data = (hashmap_element_t*)hashmap_malloc(size*sizeof(hashmap_element_t));
	if(!m->data) 
		goto err;
	
	memset(m->data,0,size*sizeof(hashmap_element_t));

	m->lock = (HANDLE) sw_mutex_create();
	if(!m->lock) 
		goto err;
		
	m->table_size = size;
	m->key_type = key_type;
	m->size = 0;

	return m;
err:
	if (m)
		sw_hashmap_destroy(m);
	return NULL;
}

/* Deallocate the hashmap */
void sw_hashmap_destroy(swhashmap_t* m)
{	
	int i;
	if(m)
	{
		lock(m);
		if(m->key_type == KEY_STRING)
		{
			for(i = 0; i< m->table_size; i++)
			{
				if(m->data[i].in_use == 1)
					hashmap_free(m->data[i].key);	
			}
		}
		hashmap_free(m->data);	
		m->size = 0;
		unlock(m);

		sw_mutex_destroy(m->lock);
		m->lock = NULL;
		hashmap_free(m);

	}
}

/*
 * Hashing function for an integer
 */
static unsigned int hashmap_digit_hash(unsigned int key)
{
	/* Robert Jenkins’ 32 bit Mix Function */
	key += (key << 12);
	key ^= (key >> 22);
	key += (key << 4);
	key ^= (key >> 9);
	key += (key << 10);
	key ^= (key >> 2);
	key += (key << 7);
	key ^= (key >> 12);

	/* Knuth’s Multiplicative Method */
	key = (key >> 3) * ( (unsigned int) 2654435761U );

	return key;// % m->table_size;
}


/*
 * Hashing function for an string
 */
static unsigned int hashmap_string_hash(char* key)
{/* if key%2==1是否需要对齐换算 */
	size_t i,l;
	unsigned long ret=0;
	unsigned short *s;
	bool bmalloc = false;
	if ( key == NULL ) return(0);
	char tmpkey[257];
	l=(strlen(key)+1)>>1;
	if ( (((unsigned long)key % 2)) == 1 )
	{
		if ( l <= 128 )
		{
			s = (unsigned short*)tmpkey;
			memcpy((char*)s, key, 2*l);//有安全判断
		}
		else
		{
			s = (unsigned short *)hashmap_malloc(2*l);
			if ( s != NULL )
			{
				memcpy((char*)s, key, 2*l);//有安全判断
				bmalloc = true;
			}
			else
				s=(unsigned short *)key;
		}
	}
	else
		s=(unsigned short *)key;

	for (i=0; i < l; ++i)
	{
		ret^=(s[i]<<(i&0x0f));
	}
	if ( bmalloc )
		hashmap_free(s);
	return ret;//(ret % m->table_size);
}

/*
 * Return the integer of the location in data
 * to store the point to the item, or MAP_FULL.
 */
static int sw_hashmap_find_empty(swhashmap_t* m, uint32_t hash)
{
	uint32_t curr;
	uint32_t fstpos;
	
	/* If full, return immediately */
	if(m->size == m->table_size) 
		return MAP_FULL;
	
	fstpos = curr = hash % m->table_size;
	/* Linear probling */
	do
	{
		if(m->data[curr].in_use != 1)
			return curr;
		curr = (curr + 1) % m->table_size;
	} while ( curr != fstpos );
	return MAP_FULL;
}

/*
 * Doubles the size of the hashmap, and rehashes all the elements
 */
static int sw_hashmap_rehash(swhashmap_t* m)
{
	int i;
	int old_size;
	hashmap_element_t* curr;

	/* Setup the new elements */
	hashmap_element_t* temp = (hashmap_element_t *)hashmap_malloc(2 * m->table_size*sizeof(hashmap_element_t));
	if(!temp) 
		return MAP_OMEM;
	
	memset(temp,0,2*m->table_size*sizeof(hashmap_element_t));

	/* Update the array */
	curr = m->data;
	m->data = temp;

	/* Update the size */
	old_size = m->table_size;
	m->table_size = 2*m->table_size;
	m->size = 0;

	/* Rehash the elements */
	for(i = 0; i < old_size; i++)
	{
		if ( curr[i].in_use == 1 )
		{
			int status = hashmap_put(m, curr[i].hash, curr[i].key, curr[i].value);
			if (status != MAP_OK)
				return status;
		}
	}

	/* free old hashmap */
	if(m->key_type == KEY_STRING)
	{
		for(i = 0; i< old_size; i++)
		{
			if(curr[i].in_use == 1)
			{
				hashmap_free(curr[i].key);	
				curr[i].in_use = 0;
			}
		}
	}
	hashmap_free(curr);

	return MAP_OK;
}

static int hashmap_put(swhashmap_t* m, uint32_t hash, void* key, void* value)
{/*调用方保证m非空key(string时)非空*/
	int index;
	/* Find a place to put our value */
	index = sw_hashmap_find_empty(m, hash);
	while(index == MAP_FULL)
	{
		if (sw_hashmap_rehash(m) == MAP_OMEM) 
			return MAP_OMEM;
		index= sw_hashmap_find_empty(m,hash);
	}
	/* Set the data */
	m->data[index].value = value;
	if( m->key_type == KEY_DIGITAL)
		m->data[index].key =(void*)key;
	else
	{
		/* free old value */
		if(m->data[index].in_use ==1 )
			hashmap_free(m->data[index].key);
		m->data[index].key =(void*)hashmap_strdup((char*)key);
		if ( m->data[index].key == NULL )
		{/* 无法申请到内存 */
			m->data[index].in_use = 0;
			return MAP_OMEM;
		}
	}	
	m->data[index].in_use = 1;
	m->data[index].hash = hash;
	m->size++; 
	return MAP_OK;
}
/*
 * Add a pointer to the hashmap with some key
 */
int sw_hashmap_put(swhashmap_t* m, void* key, void* value)
{
	int ret;
	uint32_t hash;
	if( m == NULL || ( m->key_type == KEY_STRING && (key == NULL || *(char*)key == '\0') ) )
	{
		return MAP_INVALID_KEY;
	}
	if ( m->key_type == KEY_STRING )	/* 这里可以修改代码减少在rehash时的key计算 */
		hash = hashmap_string_hash(key);
	else
		hash = hashmap_digit_hash((unsigned int)key);
	/* Lock for concurrency */
	lock(m);
	ret = hashmap_put(m, hash, key, value);
	/* Unlock */
	unlock(m);

	return ret;
}

/*
 * Get your pointer out of the hashmap with a key
 */
int sw_hashmap_get(swhashmap_t* m, void* key, void** value)
{
	uint32_t hash;
	int curr;
	int fstpos;
	int ret = MAP_MISSING;
	if( m == NULL || ( m->key_type == KEY_STRING && (key == NULL || *(char*)key == '\0') ) )
		return MAP_MISSING; 

	*value = NULL;
	/* Calculate hash key */
	if ( m->key_type == KEY_STRING )
		hash = hashmap_string_hash(key);
	else
		hash = hashmap_digit_hash((unsigned int)key);
	/* Lock for concurrency */
	lock(m);

	curr = fstpos = hash % m->table_size;
	/* Linear probing, if necessary */
	do
	{
		if( m->data[curr].in_use == 1 )
		{
			if( m->data[curr].hash == hash /* 基本上这里就可以不用往下判断了 */
			    && ( (m->key_type == KEY_STRING && m->data[curr].key != NULL &&  strcmp((char*)m->data[curr].key,(char*)key)==0)
			 		|| (m->key_type == KEY_DIGITAL && m->data[curr].key == key) ) )
			{
				*value = m->data[curr].value;
				ret = MAP_OK;
				unlock(m);
				return ret;
			}
		}
		else if ( m->data[curr].in_use == 0 )
			break;
		curr = (curr + 1) % m->table_size;
	} while ( curr != fstpos );
	/* Unlock */
	unlock(m);
	/* Not found */
	return MAP_MISSING;
}

/*
 * Remove an element with that key from the map
 */
int sw_hashmap_remove(swhashmap_t* m, void* key)
{
	int curr, fstpos;
	uint32_t hash;
	
	if( m== NULL || ( m->key_type == KEY_STRING && (key == NULL || *(char*)key == '\0') ) )
		return MAP_MISSING;

	/* Calculate hash key */
	if ( m->key_type == KEY_STRING )
		hash = hashmap_string_hash(key);
	else
		hash = hashmap_digit_hash((unsigned int)key);
	/* Lock for concurrency */
	lock(m);
	curr = fstpos = hash % m->table_size;

	/* Linear probing, if necessary */
	do 
	{
		if( m->data[curr].in_use == 1)
		{
			if( m->data[curr].hash == hash 
			    && ( (m->key_type == KEY_STRING && m->data[curr].key != NULL &&  strcmp((char*)m->data[curr].key,(char*)key)==0)
			 		|| (m->key_type == KEY_DIGITAL && m->data[curr].key == key) ) )
			{/* 需要将后续为从curr+1直到in_use为0的所有hash重新加入(这对于密集型hash表删除可能耗时比较久) */
				/* Blank out the fields */
				m->data[curr].in_use = -1;	/* 标记为删除 */
				if( m->key_type == KEY_STRING)
					hashmap_free(m->data[curr].value);

				m->data[curr].value = NULL;
				m->data[curr].key = 0;

				/* Reduce the size */
				m->size--;
				unlock(m);
				return MAP_OK;
			}
		}
		else
			break;
		curr = (curr + 1) % m->table_size;
	} while ( curr != fstpos );

	/* Unlock */
	unlock(m);

	/* Data not found */
	return MAP_MISSING;
}

/* Return the length of the hashmap */
int sw_hashmap_size(swhashmap_t* m)
{
	if(m != NULL) 
		return m->table_size;
	else 
		return 0;
}

/*
 * Get your pointer out of the hashmap by index
 */
int sw_hashmap_get_byindex(swhashmap_t* m, int index,void** key, void** value)
{	
	if( m== NULL )
		return MAP_MISSING;

	/* Lock for concurrency */
	lock(m);

	*value = NULL;
	if( index < m->table_size && m->data[index].in_use == 1)
	{
		*key = m->data[index].key;
		*value = m->data[index].value;
		unlock(m);
		return MAP_OK;
	}
	/* Unlock */
	unlock(m);
	/* Not found */
	return MAP_MISSING;
}
#ifdef _DEBUG_CONFLIT
void sw_hashmap_printf(swhashmap_t *m)
{
	if ( m == NULL )
		return;
	char buf[4096];
	int size = m->table_size;
	int i = 0;
	int curr = 0;
	int confilt = 0;
	i = snprintf(buf, sizeof(buf), "size:%d, table_size:%d, hash:frt\n", m->size, size);
	for ( ; curr < size; curr++)
	{
		if ( m->data[curr].in_use == 1 )
		{
			i += snprintf(&buf[i], i < (int)sizeof(buf) ? sizeof(buf)-i : 0, "%d:%x:%d ", curr, m->data[curr].hash, m->data[curr].hash % size);
			if ( curr != (int)(m->data[curr].hash % size) )
			{
				if ( m->key_type == KEY_STRING )
					i += snprintf(&buf[i], i < (int)sizeof(buf) ? sizeof(buf)-i : 0, "confilt:%s:%s\n", (char*)m->data[m->data[curr].hash % size].key, (char*)m->data[curr].key);
				else
					i += snprintf(&buf[i], i < (int)sizeof(buf) ? sizeof(buf)-i : 0, "confilt:%d:%d\n", (uint32_t)m->data[m->data[curr].hash % size].key, (uint32_t)m->data[curr].key);
				confilt++;
			}
			else
				i += snprintf(&buf[i], i < (int)sizeof(buf) ? sizeof(buf)-i : 0, "\n");
			if ( i > 3900 )
			{
				UTIL_LOG_INFO("%s\n", buf);
				i = 0;
				buf[i] = '\0';
			}
		}
	}
	UTIL_LOG_INFO("%s\n confilts:%d\n", buf, confilt);
}
#endif
