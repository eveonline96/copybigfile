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

#define  BLOCKSIZE  (1024*1024*2) //文件每块为2m
#define  PTHREADCNT 10             //计划线程个数
#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(-1); \
	}while(0)
//定义文件块结构信息,发送给每个线程使用
typedef struct fileblock
{
	unsigned int startfpos;
	unsigned int blocksize;
}fileblock;

//定义线程池任务队列节点
typedef struct tasknode
{
  fileblock * fpblock;
  struct tasknode  *next;
}tasknode;

//定义任务队列
typedef struct  taskqueue
{
    tasknode* head;
    tasknode* tail;
}taskqueue;

//定义线程池结构
typedef struct pthreadpool
{
	pthread_mutex_t* mutex;
	pthread_cond_t * cond;
	pthread_t * pthreads;
	tasknode * head;
	int taskcnt;      //	已接受的任务总数
	int isshutdown  ;  //	线程池是否销毁  0：不毁，1：销毁
	int tasktotalcnt; //	任务总任务数
}pthreadpool;

unsigned int  g_wake=0;  		//0:堵塞  1:唤醒
unsigned int g_hasdotaskcnt=0;  //记录已经处理完的任务数
unsigned int g_pthreadcnt=0;    //记录已经创建线程数

pthreadpool *g_pool=NULL;       //定义一个全局线程池指针
taskqueue * g_taskqueuep=NULL;  //定义指向任务队列的全局指针
fileblock * g_blockfp=NULL;     //指向文件结构全局指针


unsigned char * g_srcfaddr=NULL;    //源文件内存映射的全局指针
unsigned char * g_destfaddr=NULL;   //目标文件内存映射的全局指针
