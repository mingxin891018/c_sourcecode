#ifndef PROTO_H__
#define PROTO_H__

#define RCVPORT     "1989"
#define ERRSIZE		1024
#define PATHSIZE	1024
#define DATASIZE	10

#define DATEMAX		(60000)
#define TEST		50000

struct msg_st
{
	uint8_t flag;
	uint32_t len;
	uint8_t data[1];

}__attribute__((packed));


#endif




