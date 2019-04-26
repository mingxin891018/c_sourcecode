#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define SIZE		1024

int main(int argc,char **argv)
{
	time_t stamp;
	struct tm *tm;
	char buf[SIZE];
	int ch;
	char fmt[SIZE] = {'\0'};
	FILE *fp = stdout;

	stamp = time(NULL);
	tm = localtime(&stamp);

	while(1)	
	{// ./mydate  -y 4 -m -d  /tmp/out -M -S /tmp/a
		ch = getopt(argc,argv,"-y:mdH:MS");
		if(ch < 0)
			break;

		switch(ch)
		{
			case 1:
				if(fp == stdout)
				{
					fp = fopen(argv[optind-1],"w");			
					if(fp == NULL)
					{
						perror("fopen()");
						fp = stdout;
					}
				}
				break;

			case 'y':
				if(strcmp(optarg,"2") == 0)
					strncat(fmt,"%y ",SIZE);
				else if(strcmp(optarg,"4") == 0)
						strncat(fmt,"%Y ",SIZE);
				else 
					fprintf(stderr,"Invalid argument of -y\n");
	
				break;

			case 'm':
				strncat(fmt,"%m ",SIZE);
                break;

			case 'd':
				strncat(fmt,"%d ",SIZE);
                break;

			case 'H':
				if(strcmp(optarg,"12") == 0)
                    strncat(fmt,"%I(%P) ",SIZE);
                else if(strcmp(optarg,"24") == 0)
                        strncat(fmt,"%H ",SIZE);
                else
                    fprintf(stderr,"Invalid argument of -H\n");             
                    

                break;

			case 'M':
				strncat(fmt,"%M ",SIZE);
                break;

			case 'S':	
				strncat(fmt,"%S ",SIZE);
                break;

			default:
				exit(1);
		}

	}

	strncat(fmt,"\n",SIZE);
	strftime(buf,SIZE,fmt,tm);
	fputs(buf,fp);

	if(fp != stdout)
		fclose(fp);

	exit(0);
}


