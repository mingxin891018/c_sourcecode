#ifndef PROTO_H__
#define PROTO_H__

typedef uint8_t chnid_t;

#define DEFAULT_MGROUP		"224.2.2.2"
#define DEFAULT_RCVPORT		"1989"

#define NR_CHN		200

#define CHNID_LIST	0

#define MINCHNID	1
#define MAXCHNID	(MINCHNID+NR_CHN-1)

#define MAX_MSG		(65536-20-8)
#define MAX_DATA	(MAX_MSG-sizeof(uint8_t))
#define MAX_ENTRY	(MAX_MSG-sizeof(uint8_t))

struct msg_channel_st
{
	chnid_t chnid;	/*must be [MINCHNID,MAXCHNID]  */
	uint8_t data[1];
}__attribute__((packed));

struct list_entry_st
{
	chnid_t chnid;  /*must be [MINCHNID,MAXCHNID]  */
	uint8_t desc[1];
}__attribute__((packed));

struct msg_list_st
{	
	chnid_t chnid;  /*must be CHNID_LIST  */
	struct list_entry_st entry[1];	
}__attribute__((packed));

/*
01 xx:xxxxxxxxxxxxx\02 xxxxx:xxxxxxxxxxxxxxxx\0
01 2 3 xxxx:xxxxxx
 
1 xx:xxxxxxxxxxxxx
2 xxxxx:xxxxxxxxxxxxxxxx
3 xxxx:xxxxxx

 * */

#endif


