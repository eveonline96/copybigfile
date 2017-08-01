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
#define  PTHREADCNT 5 //计划线程个数

typedef struct  tasknode
{
	int num;
	tasknode *next;
}tasknode;

typedef struct taskqueue
{
	tasknode *head;
	tasknode *tail;
} taskqueue;

typedef struct pthreadpool
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t pthreads;
	tasknode * head;
	int taskcnt;      //	已接受的任务总数
	int isshutdown    //	线程池是否销毁  0：不毁，1：销毁
	int tasktotalcnt; //	任务总任务数
}pthreadpool;

void init_pthread_pool(int tasktotalcnt)
{
	g_pool=(pthreadpool *)malloc(1*sizeof(pthreadpool));
	while(pthreadpool==NULL)
	{
		g_pool=(pthreadpool *)malloc(1*sizeof(pthreadpool));
	}
	g_pool->mutex=NULL;
	g_pool->mutex=(pthread_mutex_t*)malloc(1*sizeof(pthread_mutex_t));
	while(g_pool==NULL)
	{
		g_pool=(pthread_mutex_t *)malloc (1*sizeof(pthread_mutex_t));
	}
	g_pool->cond=NULL;
	g_pool->cond=(pthread_cond_t *)malloc (1*sizeof(pthread_cond_t));
 	pthread_mutex_init(g_pool->mutex,NULL);
    pthread_cond_init(g_pool->cond,NULL);

	g_pool->pthreads=NULL;
	g_pool->pthreads=(pthread_t *)malloc(PTHREADCNT*sizeof(pthread_t));
	while(g_pool->pthreads==NULL)
	{
		g_pool->pthreads=(pthread_t *)malloc(PTHREADCNT*sizeof(pthread_t));
	}
	g_pool->head=NULL;
	g_pool->taskcnt=0;
	g_pool->isshutdown=0;
	g_pool->tasktotalcnt=tasktotalcnt;
	int i;
	for(i=0;i<PTHREADCNT;i++)
	{
		ret=pthread_create(&g_pool->pthreads[i],NULL,Run,NULL);
		while(ret!=0)
		{
			ret=pthread_create(&g_pool->pthreads[i],NULL,Run,NULL);
		}
	}
}

void clean_pthread_pool()
{

}

int g_pthreadcnt=0;


int main(int argc, char const *argv[])
{
	init_pthread_pool(100);
	while(g_pthreadcnt!=PTHREADCNT){}
	clean_pthread_pool();
	return 0;
}