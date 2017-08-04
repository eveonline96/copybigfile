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
	struct tasknode * next;
}tasknode;

typedef struct taskqueue
{
	 tasknode *head;
	 tasknode *tail;
} taskqueue;

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

pthreadpool * g_pool=NULL;
taskqueue * g_taskqueuep=NULL;
int g_wake=0;
unsigned int g_pthreadcnt=0;

//初始化队列
void init_queue(taskqueue*qtask)
{
	qtask->head=NULL;
	qtask->tail=NULL;
}
//进队操作
void push_queue(taskqueue *qtask)
{
	tasknode *new_tnode=(tasknode *)malloc(1*sizeof(tasknode));
	new_tnode->num=100;
	new_tnode->next=NULL;
	if (qtask->head==NULL && qtask->tail==NULL)
	{
		qtask->head=new_tnode;
		qtask->tail=new_tnode;
	}
	else
	{
		qtask->tail->next=new_tnode;
		qtask->tail=new_tnode;
	}
}

//创建任务队列
void init_task_list()
{
	g_taskqueuep=(taskqueue*)malloc(1*sizeof(taskqueue));
	if (g_taskqueuep==NULL)
	{
		perror("init task list fail");
	}
	init_queue(g_taskqueuep);
	int i;
	for(i=0;i<g_pool->tasktotalcnt;i++)
	{
		push_queue(g_taskqueuep);
		g_pool->taskcnt++;
		pthread_cond_signal(g_pool->cond);  //唤醒
	}
}

//判断队列是否为空
int isn_empty_queue(taskqueue *qtask)
{
	int isempty=1;
	if (qtask->head==NULL && qtask->tail==NULL)
	{
		isempty=0;
	}
	return isempty;
}
//出队列操作
tasknode * pop_queue(taskqueue * qtask)
{
	tasknode * returnnode=NULL;
	if (isn_empty_queue(qtask))
	{
		returnnode=qtask->head;
		qtask->head=qtask->head->next;
		if (qtask->head==NULL)
		{
			qtask->tail=NULL;
		}
		returnnode->next=NULL;
	}
	return returnnode;
}
//释放队列
void free_queue(taskqueue *qtask)
{
	if (qtask!=NULL&&qtask->head==NULL &&qtask->tail==NULL)
	{
		free(qtask);
		qtask=NULL;
	}
}

//用线程实现函数
// void* print_num(void * arg)
// {
// 	tasknode * a=(tasknode*)arg;
// 	printf("%d\n",a->num);
// 	return NULL;
// }

//执行函数
void* Run(void* arg)
{
	while(1)
	{
		pthread_mutex_lock(g_pool->mutex);//刚开始无任务执行,堵塞线程 
		g_pthreadcnt++;                   //记录成功创建的线程数
		if (g_pool->taskcnt==0 && g_pool->isshutdown==0)
		{
			pthread_cond_wait(g_pool->cond,g_pool->mutex);
		}
		g_pool->head=pop_queue(g_taskqueuep);
		pthread_mutex_unlock(g_pool->mutex);  //修改公共资源

		if (g_pool->head!=NULL)
		{
			//print_num((void*)&g_pool->head);
			printf("%d\n",pthread_self());
			printf("%d\n",g_pool->head->num);
			free(g_pool->head);
			g_pool->head=NULL;
			g_pool->taskcnt--;
		}
	}
}


//线程池初始化
void init_pthread_pool(unsigned int tasktotalcnt)
{
	g_pool=(pthreadpool *)malloc(1*sizeof(pthreadpool));
	while(g_pool==NULL)
	{
		g_pool=(pthreadpool *)malloc(1*sizeof(pthreadpool));
	}

   g_pool->mutex=NULL;
   g_pool->mutex=(pthread_mutex_t*)malloc(1*sizeof(pthread_mutex_t));
   while(g_pool->mutex==NULL)
   {
      g_pool->mutex=(pthread_mutex_t*)malloc(1*sizeof(pthread_mutex_t));
   }

   g_pool->cond=NULL;
   g_pool->cond=(pthread_cond_t*)malloc(1*sizeof(pthread_cond_t));
   while(g_pool->cond==NULL)
   {
      g_pool->cond=(pthread_cond_t*)malloc(1*sizeof(pthread_cond_t));
   }

   pthread_mutex_init(g_pool->mutex,NULL);
   pthread_cond_init(g_pool->cond,NULL);

   g_pool->pthreads=NULL;
   g_pool->pthreads=(pthread_t *)malloc(sizeof(pthread_t)*PTHREADCNT);
   while(g_pool->pthreads==NULL)
   {
      g_pool->pthreads=(pthread_t *)malloc(sizeof(pthread_t)*PTHREADCNT);
   }
   g_pool->head=NULL;
   g_pool->taskcnt=0;
   g_pool->isshutdown=0;
   g_pool->tasktotalcnt=tasktotalcnt;

   int i,ret;
   for(i=0;i<PTHREADCNT;i++)
   {
   	 ret=pthread_create(&(g_pool->pthreads[i]),NULL,Run,NULL);
   	 while(ret!=0)
   	 {
   	 	ret=pthread_create(&(g_pool->pthreads[i]),NULL,Run,NULL);
   	 }
   }
   while(g_pthreadcnt!=PTHREADCNT){}
}

int main(int argc, char const *argv[])
{
	init_pthread_pool(10);
	init_task_list();
	return 0;
}


