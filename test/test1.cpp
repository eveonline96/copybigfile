#include  "define_header_file.h"
/*
多个线程对进程公共资源竞争
线程同步互斥
线程互斥操作:
pthread_mutex_t :线程互斥变量类型(原子类型)
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
        const pthread_mutexattr_t *restrict attr);
动态初始化互斥变量,互斥变量属性为默认值restrict_attr 为空,必须调用函数释放
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
静态初始化互斥变量,不用释放
int pthread_mutex_destroy(pthread_mutex_t *mutex)
释放互斥变量


int pthread_mutex_lock(pthread_mutex_t *mutex);
加锁,如果已经被锁住,线程堵塞
int pthread_mutex_trylock(pthread_mutex_t *mutex);
加锁,如果已经被锁住,线程立即返回
int pthread_mutex_unlock(pthread_mutex_t *mutex);
解锁
*/
pthread_mutex_t  g_mutex=PTHREAD_MUTEX_INITIALIZER;
int g_data=1;
void  input_fun()
{
    while(1)
    {
        pthread_mutex_lock(&g_mutex);
        //printf("pthread  is running\n");
        //sleep(1);
        //pthread_mutex_unlock(&g_mutex);

        printf("thread1  g_data=%d\n",g_data);
        g_data++;//修改公共资源加锁保护
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
        if(g_data>=10)
         break;
    }
}

void  input_fun2()
{
    while(1)
    {
        pthread_mutex_lock(&g_mutex);
        //printf("pthread  is running\n");
        //sleep(1);
        pthread_mutex_unlock(&g_mutex);

        printf("thread2  g_data=%d\n",g_data);
        g_data++;
        //pthread_mutex_unlock(&g_mutex);
         sleep(1);
        if(g_data>=10)
         break;
    }
}
int main(int argc,const char* argv[])
{
    pthread_t  pthreadid1,pthreadid2,pthreadid3;
    int ret=pthread_create(&pthreadid1,NULL,(void*)input_fun,NULL);
    while(ret!=0)
    {
        ret=pthread_create(&pthreadid1,NULL,(void*)input_fun,NULL);
    }   
    ret=pthread_create(&pthreadid2,NULL,(void*)input_fun2,NULL);
    while(ret!=0)
    {
        ret=pthread_create(&pthreadid2,NULL,(void*)input_fun2,NULL);
    }   
    pthread_join(pthreadid1,NULL); 
    pthread_join(pthreadid2,NULL); 
    return  0;
}

