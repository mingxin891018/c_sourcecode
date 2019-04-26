#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

int main(int argc,char **argv)
{
	struct passwd *cur;

	if(argc < 2)
	{
		fprintf(stderr,"Usage...\n");
		exit(1);
	}

	cur = getpwuid(atoi(argv[1]));
	
	puts(cur->pw_name);


	exit(0);

}


