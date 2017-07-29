//文件处理代码
//计算文件大小
unsigned int file_size(unsigned int fd)
{
	unsigned int f_size=lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	return f_size;
}

//文件分块数
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

//非整数块时，计算出文件剩余大小
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

//源文件映射地址
unsigned char * srcfile_mapaddr(unsigned int fd)
{
	unsigned int f_size=file_size(fd);
	g_srcfaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (g_srcfaddr==NULL)
	{
		ERR_EXIT("srcfile map address fail");
	}
	return g_srcfaddr;
}

//源文件大小已知，目标文件映射地址
unsigned char * destfile_mapaddr(unsigned int srcfd,unsigned int destfd)
{
	unsigned int f_size=file_size(srcfd);
	lseek(destfd,f_size,SEEK_SET);
	write(destfd, " ",1);
	g_destfaddr=(unsigned char *)mmap(NULL,f_size,PROT_READ|PROT_WRITE,MAP_SHARED,destfd,0);
	if (g_destfaddr==NULL)
	{
		ERR_EXIT("destfile map address fail");
	}
	return g_destfaddr;
}

//释放文件内存映射
void file_munmap(unsigned int srcfd)
{
	unsigned int f_size=file_size(srcfd);
	if (g_srcfaddr!=NULL)
	{
		munmap(g_srcfaddr,f_size);
	}
	if (g_destfaddr!=NULL)
	{
		munmap(g_destfaddr,f_size);
	}
}

//计算文件分块的结构
fileblock * file_block(unsigned int fd)
{
	unsigned int f_blockcnt=file_blockcnt(fd);
	unsigned int f_remsize=file_remsize(fd);
	fileblock * f_blockarr=(fileblock*)malloc(f_blockcnt*sizeof(fileblock));
	if(f_blockarr==NULL)
	 {
	 	ERR_EXIT("file block malloc fail");
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

//用线程实现函数
void* copy_file(void * arg)
{
	fileblock * b_struct=(fileblock*)arg;
	memcpy((void *)&g_destfaddr[b_struct->startfpos],(void *)&g_srcfaddr[b_struct->startfpos],b_struct->blocksize);
	return NULL;
}

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


//执行函数
void* Run(void* arg)
{
	while(1)
	{
		pthread_mutex_lock(g_pool->mutex);
		 //刚开始无任务执行,堵塞线程
		g_pthreadcnt++;//记录成功创建的线程数
		if (g_pool->taskcnt==0 && g_pool->isshutdown==0)
		{
			pthread_cond_wait(g_pool->cond,g_pool->mutex);
		}
		g_pool->head=pop_queue(g_taskqueuep);
		pthread_mutex_unlock(g_pool->mutex);  //让别的线程继续
		if (g_pool->head!=NULL)
		{
			copy_file((void*)&g_pool->head->fpblock);
			free(g_pool->head);
			g_pool->head=NULL;
			g_pool->taskcnt--;
			printf("g_pool->taskcnt=%d\n",g_pool->taskcnt);
			//cout<<"g_pool->taskcnt"<<g_pool->taskcnt<<endl;
			g_finish_task++;  //记录已经完成一个任务 
			if(g_pool->taskcnt==0 && g_finish_task==g_pool->tasktotalcnt)
			{
				g_wake=1;
		        if(g_blockfp!=NULL)
		        {
		           free(g_blockfp);
		           g_blockfp=NULL;
		        }
			}
		}
		  
		usleep(1);//防止总是让一个线程做
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
//
