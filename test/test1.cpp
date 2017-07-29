#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <cstring>
#include <string>
using namespace std;


int main(int argc, char const *argv[])
{
	//char *ch1=(char*)malloc(100*sizeof(char));
	// char ch1[100];
	// char *ch2="new";
	// char ch3[100];
	// scanf("%s",&ch1);
	// strcpy(ch3,ch2);
	// strcat(ch3,ch1);
	// printf("%s\n",ch3);

	char ch1[100];
	char ch2[10]="new_";
	scanf("%s",&ch1);
	//strcat(ch2,ch2);
	open(ch1,O_CREAT|O_EXCL|O_RDWR,0777);
	printf("open ok\n");
	//printf("%s\n",ch2);

	// string str1=" ";
	// cin>>str1>>endl;
	// string str2="new";
	// string str3=str2+str1;
	// cout<<str3<<endl;
	return 0;
}
