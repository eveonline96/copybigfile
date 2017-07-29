#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
unsigned char *g_srcaddr=NULL;
unsigned char *g_destaddr=NULL;
unsigned int g_size=0;

unsigned int file_size(unsigned int fd)
{
	unsigned int f_size=lseek(fd,0,SEEK_END);
	g_size=f_size;
	lseek(fd,0,SEEK_SET);
	return f_size;
}

// unsigned char *srcfile_mapaddr(unsigned int fd)
// {
// 	unsigned int f_size=file_size(fd);
// 	g_srcaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
// 	if (g_srcaddr==NULL)
// 	{
// 		perror("srcfile map address fail");
// 		exit(-1);
// 	}
// 	return g_srcaddr;
// }

// unsigned char *destfile_mapaddr(unsigned int srcfd,unsigned int destfd)
// {
// 	unsigned int f_size=file_size(srcfd);
// 	lseek(destfd,f_size,SEEK_SET);
// 	g_destaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,destfd,0);
// 	if (g_destaddr==NULL)
// 	{
// 		perror("destfile map address fail");
// 		exit(-1);
// 	}
// 	return g_destaddr;
// }

void mmap_addr(unsigned int srcfd,unsigned int destfd)
{
	unsigned int f_size=file_size(srcfd);
	g_srcaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,srcfd,0);
	
	lseek(destfd,f_size,SEEK_SET);
	write(destfd," ",1);
	g_destaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,destfd,0);
}
int main(int argc, char const *argv[])
{
	printf("Please input copyfile name\n");
	char srcfname[100];
	char ch1[100]="new_";
	scanf("%s",&srcfname);
	strcat(ch1,srcfname);
	int  srcfd=open(srcfname,O_RDWR);
	int destfd=open(ch1,O_CREAT|O_EXCL|O_RDWR,0777);
	// unsigned char * srcadd=srcfile_mapaddr(srcfd);
	// unsigned char *destadd=destfile_mapaddr(srcfd,destfd);
	mmap_addr(srcfd, destfd);
	printf("%u\n",g_destaddr[0]);
	//printf("%u\t%u\t%d\n",g_destaddr,g_srcaddr,g_size);
	//memcpy((void*)&g_destaddr[0],(void*)&g_srcaddr[0],g_size);	
	//memcpy((void*)&g_destaddr,(void*)&g_srcaddr,g_size);	
	return 0;
}
