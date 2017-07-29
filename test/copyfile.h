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
#define  TASKSIZE 20  //计划每批执行的任务数
#define  PTHREADCNT 5 //计划线程个数
#define  BLOCKSIZE  (1024*1024*2)//每个文件块大小2M
//定义文件块结构信息,发送给每个线程使用
typedef struct fileblock
{
	unsigned char* srcfileaddres;
	unsigned char* destfileaddres;
	unsigned int startfilepos;
	unsigned int blocksize;
}fileblock;

//定义线程池任务队列节点
typedef struct tasknode
{
  fileblock *fpblock;
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
	pthread_mutex_t * mutex;//互斥锁
	pthread_cond_t  *cond;//条件变量
	pthread_t *pthreads;//指向线程id集合指针   
    tasknode * head;//任务起始位置
	unsigned int taskcnt;//已经接收任务数量
	unsigned int isshutdown;//线程池是否可以销毁 0:不能销毁 1:销毁
	unsigned int tasktotalcnt;//任务总数量
}pthreadpool;


//主线程唤醒标志位
unsigned int  g_ismainwake=0; //0:堵塞  1:唤醒
//记录已经处理完的任务数
unsigned int g_hasdotaskcnt=0;
//记录计划执行任务批次数
unsigned int  g_totaltaskNO=0;
//记录已经创建线程数
unsigned int g_pthreadcnt=0;
//记录每批次分配的任务数
unsigned int g_taskcntper=0;
//定义一个全局线程池指针
pthreadpool *g_pool=NULL;
//定义指向任务队列的全局指针
taskqueue * g_taskqueuep=NULL;
//指向文件结构全局指针
fileblock * g_fileblockfp=NULL;
//源文件内存映射的全局指针
unsigned char * g_srcfilestartp=NULL;
//目标文件内存映射的全局指针
unsigned char * g_destfilestartp=NULL;
