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


//初始化队列
void init_queue(taskqueue*qtask)
{
	qtask->head=NULL;
	qtask->tail=NULL;
}
//进队操作
void push_queue(taskqueue *qtask,fileblock*fblock)
{
	tasknode *new_tnode=(tasknode *)malloc(1*sizeof(tasknode));
	new_tnode->fpblock=fblock;
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
void init_task_list(unsigned int fd)
{
	g_taskqueuep=(taskqueue*)malloc(1*sizeof(taskqueue));
	if (g_taskqueuep==NULL)
	{
		ERR_EXIT("init task list fail");
	}
	init_queue(g_taskqueuep);
	g_blockfp=file_block(fd);
	int i;
	for(i=0;i<g_pool->tasktotalcnt;i++)
	{
		push_queue(g_taskqueuep,&g_blockfp[i]);
		g_pool->taskcnt++;
		cout<<"taskcnt="<<g_pool->taskcnt<<endl;   //记录
		pthread_cond_signal(g_pool->cond);  //唤醒
	}
}


//判断队列是否为空
int is_empty_queue(taskqueue *qtask)
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
	if (is_empty_queue(qtask))
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


void *Run(void *arg)
{
	while(1)
	{
		pthread_mutex_lock(g_pool->mutex);
		g_pthreadcnt++;
		if (g_pool->taskcnt==0&& g_gool->isshutdown==0)
		{
			pthread_cond_wait(g_pool->cond,g_pool->mutex);
		}
		g_pool->head=pop_queue(g_taskqueuep);
		pthread_mutex_unlock(g_pool->mutex);
		if (g_pool->head!=NULL)
		{
			g_pool->head=NULL;
			g_pool->taskcnt--;
			printf("%d\n",g_gool->tasktotal);
				
		}
	}
}

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