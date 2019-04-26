#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <string.h>

#define DELIMS	" \t\n"

struct cmd_st
{
	glob_t globres;
};

void prompt(void)
{
	printf("[mysh-0.1]$ ");
}

void parse(char *line,struct cmd_st *res)
{// "ls  ./a*.c    -l     -a     -i /etc" -> "ls" "-l" "-a" ...
	
	char *tok;
	int i = 0;

	while(1)
	{
		tok = strsep(&line,DELIMS);
		if(tok == NULL)
			break;
		if(tok[0] == '\0')
			continue;			

		glob(tok,GLOB_NOCHECK|GLOB_APPEND*i,NULL,&res->globres);
		i = 1;	
	}
}

int main()
{
	char *linebuf = NULL;
	size_t linesize = 0;
	struct cmd_st cmd;
	pid_t pid;

	while(1)
	{
		//打印提示符
		prompt();

		//接收输入
		
		if(getline(&linebuf,&linesize,stdin) < 0)
			break;

		//分析字符串
		parse(linebuf,&cmd);		

		// do sth 
		if(/*内部命令*/ 0 )
		{		}
		else		// 外部命令
		{
			pid = fork();
			if( pid < 0)
			{
				perror("fork()");
				exit(1);
			}

			if(pid == 0)	// child
			{
				execvp(cmd.globres.gl_pathv[0],cmd.globres.gl_pathv);
				perror("execvp()");
				exit(1);
			}
			wait(NULL);
		}
	}

	exit(0);
}





