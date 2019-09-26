#include <stdio.h>
int main(void)
{
	char *p = (char *)"\xE6\xB5\x8B\xE8\xAF\x95";
	printf("%s\n", p);
	return 0;
}
