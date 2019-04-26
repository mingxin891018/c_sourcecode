#include <stdio.h>
#include <stdlib.h>
#include <shadow.h>
#include <unistd.h>

int main(int argc,char **argv)
{
	struct spwd *cur;
	char *inputpass,*crypted_pass;
	
	if(argc < 2)
	{
		fprintf(stderr,"Usage....\n");
		exit(1);
	}
	
	inputpass = getpass("PASSWD:");	
	/*if error*/

	cur = getspnam(argv[1]);
	/*if error*/
	
	crypted_pass = crypt(inputpass,cur->sp_pwdp);
	/*if error*/

	if(strcmp(crypted_pass,cur->sp_pwdp) == 0)
		puts("OK!");
	else
		puts("FAILED!");


	exit(0);
}


