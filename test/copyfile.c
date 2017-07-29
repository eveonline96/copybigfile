#include "copyfile.h"
#include "pool.h"

/*
线程池=创建线程+线程控制+任务队列
unsigned int 表示范围大一倍
C++禁止将void指针随意赋值给其他指针。pthread_create时c++会报错
*/
int main(int argc, char *argv[])
{
	unsigned int initpthreadcnt=PTHREADCNT;
	int srcfd=open("java.avi",O_RDWR);
	if(srcfd==-1)
	{
		output_sys_errmsg("main open srcfd:");
		exit(-1);
	}
	int destfd=open("newjava.avi",O_CREAT|O_EXCL|O_RDWR,0777);
  if(destfd==-1)
  {
    output_sys_errmsg("main open destfd:");
    exit(-1);
  }
  unsigned int fblockcnt=get_file_block_cnt(srcfd);
  get_srcfile_map_addres(srcfd);
  get_destfile_map_addres(srcfd,destfd);
	init_pthread_pool(initpthreadcnt,fblockcnt);
	init_task_list(srcfd);
	while(g_ismainwake==0)
		   ;	
	set_srcfile_munmap(srcfd);
  set_destfile_munmap(srcfd);  
	clean_pthread_pool(initpthreadcnt);	
  free_queue_point(g_taskqueuep);
	return 0;
}
