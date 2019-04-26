#ifndef PROTO_H__
#define PROTO_H__

enum
{
	MSG_PATH=1,
	MSG_S2C,
};

#define PATHSIZE	1024
#define DATASIZE	1024

struct msg_path_st
{
	long mtype;				/*must be MSG_PATH*/
	char path[PATHSIZE];	/* ASCIIZ 带尾0的串 */
};

struct msg_s2c_st
{
	long mtype;				/*must be MSG_S2C*/
	int datalen;
/*	
 *	datalen > 0:	data
 *			= 0:	eot
 *			< 0:	-errno
 * */	

	char data[DATASIZE];
};

#endif



