#include <stdio.h>
int main(int argc, char const *argv[])
{
	char  *str ="abc";
	int i=2;
	printf("%c\n",str[i]);
	printf("%d\n",&str[i]);
	printf("%d\n",&str);
		i=1;
	printf("%c\n",str[i]);
	printf("%d\n",&str[i]);
	printf("%d\n",&str);
	return 0;
}
