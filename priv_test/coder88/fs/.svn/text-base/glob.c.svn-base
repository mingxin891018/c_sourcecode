#include <stdio.h>
#include <stdlib.h>
#include <glob.h>

#define PAT		"/etc/a*.conf"

int func(const char *epath, int eerrno)
{
	fprintf(stderr,"%s:%s\n",epath,strerror(eerrno));
}

int main()
{
	glob_t globres;
	int err,i;
	
	err = glob(PAT,0,NULL/*func*/,&globres);
	if(err)
	{
		fprintf(stderr,"glob() errno.\n");
		exit(1);
	}

	for(i = 0 ; i < globres.gl_pathc ; i++)
		puts(globres.gl_pathv[i]);

	globfree(&globres);

	exit(0);
}


