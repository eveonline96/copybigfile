实现线程池高效复制大文件

线程池=线程+队列+控制   复制文件=地址+文件映射

结构体：
文件块  fileblock
{
	unsigned int startfpos;   //起始位置
	unsigned int blocksize;   //文件块大小
}

任务节点  tasknode
{
	fileblock * fpblock;     //文件块指针
	struct tasknode *next;   
}

任务队列   taskqueue
{
	tasknode *head;
	tasknode *tail;
}

线程池   pthreadpool
{
	pthread_mutex_t * mutex;   //互斥锁
	pthread_cond_t  *cond;     //条件变量
	pthread_t *pthreads;       //指向线程id集合指针   
    tasknode * head;           //任务起始位置
	unsigned int taskcnt;      //已经接收任务数量
	unsigned int isshutdown;   //线程池是否可以销毁 0:不能销毁 1:销毁
	unsigned int tasktotalcnt; //任务总数量
}

任务队列函数：
创建任务队列    	  void init_tasklist(unsigned int fd)
    初始化链表队列    void init_queue(taskqueue*qtask,fileblock * fblock)
    进队操作          void push_queue(taskqueue * qtask,fileblock *fblock)
出队操作         	  tasknode *pop_queue(taskqueue *qtask)
	判断队列是否为空  int  no_empty_queue(taskqueue *qtask)
释放队列              void  free_queue(taskqueue  * qtask)  
释放文件结构          void   free_blockfp(fileblock * blockfp)


线程池的函数:
初始化线程池      void init_pthreadpool(unsigned int tasktotalcnt)
	线程池的任务  void* Run(void *arg)
释放线程池        void clean_pthreadpool()


文件处理函数：
计算文件大小                      unsigned int file_size(unsigned int fd)
文件分块数	                      unsigned int file_blockcnt(unsigned int fd)
非整数块时，计算出文件剩余大小    unsigned int file_remsize(unsigned int fd)
源文件映射地址     				  unsigned char * srcfile_mapaddr(unsigned int fd)
源文件大小已知，目标文件映射地址  unsigned char * destfile_mapaddr(unsigned int srcfd,unsigned int destfd)
释放文件内存映射				  void file_munmap(unsigned int srcfd)
计算文件分块的结构				  fileblock * file_block(unsigned int fd)

全局定义变量：
#define  PTHREADCNT 5             //计划线程个数
#define  BLOCKSIZE  (1024*1024*2) //文件每块为2m

unsigned int  g_ismainwake=0;   //0:堵塞  1:唤醒
unsigned int g_hasdotaskcnt=0;  //记录已经处理完的任务数
unsigned int g_pthreadcnt=0;    //记录已经创建线程数

pthreadpool *g_pool=NULL;  //定义一个全局线程池指针
taskqueue * g_taskqueuep=NULL;  //定义指向任务队列的全局指针
fileblock * g_blockfp=NULL;   //指向文件结构全局指针

unsigned char * g_srcfilestartp=NULL;   //源文件内存映射的全局指针
unsigned char * g_destfilestartp=NULL;   //目标文件内存映射的全局指针



time ./cf
计算时间

       单线程       系统命令     多线程   
11.5M  0.358S                    0.255s                 
1.36G  2min15592S    1min        49S                        

