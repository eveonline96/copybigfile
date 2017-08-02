#include <stdio.h>
using namespace std;

int floor(int n)
{
	if (n==1)
	{
		return 1;
	}
	if (n==2)
	{
		return 2;
	}
	if (n==3)
	{
		return 3;
	}
	if (n>3)
	{
		return floor(n-1)+floor(n-2)+floor(n-3);
	}
}
int main(int argc, char const *argv[])
{
	int n=15;
	int a=floor(n);
	printf("%d\n",a );
	return 0;
}
