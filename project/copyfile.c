#include "copyfile.h"
#include "pool.h"

/*
线程池=创建线程+线程控制+任务队列
unsigned int 表示范围大一倍
C++禁止将void指针随意赋值给其他指针。pthread_create时c++会报错
*/
int main(int argc, char *argv[])
{
	//printf("Please input copyfile name\n");
	char srcfname[100]="ape.avi";
	char ch1[100]="new_";
	//scanf("%s",&srcfname);
	strcat(ch1,srcfname);
	int  srcfd=open(srcfname,O_RDWR);
	if (srcfd==-1)
	{
		ERR_EXIT("open srcfd fail");
	}
	int destfd=open(ch1,O_CREAT|O_RDWR,0777);
	if(destfd==-1)
	{
	  ERR_EXIT("open destfd fail");
	}

	unsigned int fblockcnt=file_blockcnt(srcfd);
	srcfile_mapaddr(srcfd);
	destfile_mapaddr(srcfd,destfd);
	init_pthread_pool(fblockcnt);
	init_task_list(srcfd);
	while(g_wake==0){}
		//printf("begin destory\n");

	file_munmap(srcfd);
	//printf("munmap destory\n");


	clean_pthreadpool(PTHREADCNT);
	//printf("pthreadpool destory\n");


	free_queue(g_taskqueuep);
	//printf("g_taskqueuep destory\n");
	
	free_blockfp(g_blockfp);
	//printf("g_blockfp destory\n");

	return 0;
}
