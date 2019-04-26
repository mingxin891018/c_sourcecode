#ifndef  TS_H_
#define  TS_H_

#define NAMESIZE	1024
#define FILENAME	"387_0604_195948.ts"
#define PACKET 		188	

//频道媒体信息
typedef struct pmt_list{
	unsigned short stream_type;
	unsigned short elementary_PID;
}program_list_st;

//频道信息
typedef struct pmt{
	unsigned char discriptor_buffer[2][PACKET];
	unsigned int program_number;
	unsigned int pmt_pid;
	
	unsigned int discriptor_length[2];
	unsigned short running_status;
	unsigned short free_ca_mode;
	
	program_list_st program_list[10];
	unsigned int number_program_list;
}program_st;

//TS流文件内容描述
typedef struct ts{
	program_st programs[5];
	unsigned int number_program;

	FILE *fd;
	ssize_t file_size;
	
	unsigned char buffer[PACKET];
	unsigned char pmt_buffer[PACKET];
	unsigned char sdt_buffer[PACKET];
	unsigned char video_or_audio_buffer[PACKET];
}ts_st;

int ts_rewind(FILE *fp); //查找最近同步点

int read_ts_packet(FILE *fp,unsigned char *packet,int len); //读取ts文件

int parse_ts(const unsigned char *buffer,int file_size);//解析ts文件

int parse_pat(const unsigned char *buffer,ts_st *ts);//找出ts文件中的pat表

int find_pmt(unsigned short pmt_pid,ts_st *ts);//找出pmt表

int parse_pmt(const unsigned char *buffer,int len,ts_st *ts,int temp);//解析pmt表

int find_sdt(const unsigned char *buffer,ts_st *ts);//找出sdt表

int parse_sdt(const unsigned char *buffer,ts_st *ts);//解析sdt表

int ts_print(ts_st *ts);


#endif
