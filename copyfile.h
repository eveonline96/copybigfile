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
using namespace std;

#define PTHREADCNT 5  //计划线程个数
#define BLOCKSIZE (1024*1024*2)  //文件每块2m
#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(-1); \
	}while(0)
//定义文件块结构信息
typedef struct fileblock
{
	unsigned char *srcfaddr;
	unsigned char *destfaddr;
	unsigned int startfpos;
	unsigned int blocksize;
}fileblock;
//定义节点
typedef struct tasknode
{
	fileblock * fpblock;
	struct tasknode *next;
}tasknode;
//定义队列
typedef struct taskqueue
{
	tasknode* head;
	tasknode * tail;
}taskqueue;
//定义线程池
typedef struct pthreadpool
{
	pthread_mutex_t  *mutex;
	pthread_cond_t  *cond;
	pthread_t  * pthreads;
	tasknode * head;
	unsigned int taskcnt;
	unsigned int isshutdown;
	unsigned int tasktotalcnt;
}pthreadpool;

pthreadpool * g_pool=NULL;
taskqueue * g_taskqueuep=NULL;  //任务队列全局指针
fileblock * g_blockfp=NULL;      //文件全局指针
unsigned char *g_srcfaddr=NULL;
unsigned char *g_destfaddr=NULL;
int g_wake=0;   //0:堵塞  1:唤醒
int g_finish_task=0;
unsigned int g_pthreadcnt=0;  //记录已经创建线程数

