#include "copyfile.h"
#include "pool.h"

/*
线程池=创建线程+线程控制+任务队列
unsigned int 表示范围大一倍
C++禁止将void指针随意赋值给其他指针。pthread_create时c++会报错
*/
int main(int argc, char const *argv[])
{

	printf("Please input copyfile name\n");
	char srcfname[100];
	char ch1[100]="new_";
	scanf("%s",&srcfname);
	strcat(ch1,srcfname);
	int  srcfd=open(srcfname,O_RDWR);
	if (srcfd==-1)
	{
		ERR_EXIT("open srcfd fail");
	}
	int destfd=open(ch1,O_CREAT|O_EXCL|O_RDWR,0777);
	if(destfd==-1)
	{
	  ERR_EXIT("open destfd fail");
	}
	unsigned int fblockcnt=file_blockcnt(srcfd);
	printf("%d\n", fblockcnt);
	srcfile_mapaddr(srcfd);
	destfile_mapaddr(srcfd,destfd);
	init_pthread_pool(fblockcnt);
	init_task_list(srcfd);
	while(g_wake==0){}
		cout<<"begin destory"<<endl;
	file_munmap(srcfd);
	free_queue(g_taskqueuep);
	return 0;
}
