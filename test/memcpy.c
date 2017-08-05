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
#define BLOCKSIZE  (1024*1024*2)

unsigned char *g_srcaddr=NULL;
unsigned char *g_destaddr=NULL;
unsigned int g_blocksize=0;

typedef struct fileblock
{
	unsigned int startfpos;
	unsigned int blocksize;
}fileblock;

//文件处理
unsigned int file_size(unsigned int fd)
{
	unsigned int f_size=lseek(fd,0,SEEK_END);
	g_blocksize=f_size;
	lseek(fd,0,SEEK_SET);
	return f_size;
}

unsigned int file_blockcnt(unsigned int fd)
{
	unsigned int f_size=file_size(fd);
	unsigned int f_blockcnt=f_size/BLOCKSIZE;
	unsigned int f_remsize=f_size%BLOCKSIZE;
	if (f_remsize>0)
	{
		f_blockcnt+=1;
	}
	return f_blockcnt;
}

unsigned int file_remsize(unsigned int fd)
{
	unsigned int f_size=file_size(fd);
	unsigned int f_remsize=f_size%BLOCKSIZE;
	if (f_remsize>0)
	{
		return f_remsize;
	}
	else
	{
		return 0;
	}
}

unsigned char *srcfile_mapaddr(unsigned int fd)
{
	unsigned int f_size=file_size(fd);
	g_srcaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (g_srcaddr==NULL)
	{
		perror("srcfile map address fail");
		exit(-1);
	}
	return g_srcaddr;
}

unsigned char *destfile_mapaddr(unsigned int srcfd,unsigned int destfd)
{
	unsigned int f_size=file_size(srcfd);
	lseek(destfd,f_size,SEEK_SET);
	write(destfd," ",1);
	g_destaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,destfd,0);
	if (g_destaddr==NULL)
	{
		perror("destfile map address fail");
		exit(-1);
	}
	return g_destaddr;
}

void file_munmap(unsigned int srcfd)
{
	unsigned int f_size=file_size(srcfd);
	if (g_srcaddr)
	{
		munmap(g_srcaddr,f_size);
	}
	if (g_destaddr)
	{
		munmap(g_destaddr,f_size);
	}
}

fileblock *file_block(unsigned int fd)
{
	unsigned int f_blockcnt=file_blockcnt(fd);
	unsigned int f_remsize=file_remsize(fd);
	fileblock * f_blockarr=(fileblock*)malloc(f_blockcnt*sizeof(fileblock));
	if(f_blockarr==NULL)
	{
		perror("malloc fail");
		exit(-1);
	}
	int i;
	for(i=0;i<f_blockcnt-1;i++)
	{
		f_blockarr[i].startfpos=i*BLOCKSIZE;
		f_blockarr[i].blocksize=BLOCKSIZE;
	}
	f_blockarr[i].startfpos=i*BLOCKSIZE;
	f_blockarr[i].blocksize=f_remsize>0?f_remsize:BLOCKSIZE;
	return f_blockarr;
}


int main(int argc, char const *argv[])
{
	//printf("Please input copyfile name\n");
	char srcfname[100]="ape.avi";
	char ch1[100]="new_";
	//scanf("%s",&srcfname);
	strcat(ch1,srcfname);
	int  srcfd=open(srcfname,O_RDWR);
	if(srcfd==-1)
	{
		perror("main open srcfd:");
		exit(-1);
	}
	int destfd=open(ch1,O_CREAT|O_RDWR,0777);
	if(destfd==-1)
	{
		perror("main open srcfd:");
		exit(-1);
	}
	srcfile_mapaddr(srcfd);
	destfile_mapaddr(srcfd,destfd);
	unsigned int f_blockcnt=file_blockcnt(srcfd);
	fileblock * b_struct=file_block(srcfd);
	int i;     
	for (i = 0; i < f_blockcnt-1; ++i)  //小文件f_blockcnt=0不满足条件直接退出
	{
		memcpy((void*)&g_destaddr[b_struct[i].startfpos],(void*)&g_srcaddr[b_struct[i].startfpos],b_struct[i].blocksize);			
	}
	file_munmap(srcfd);
	return 0;
}
