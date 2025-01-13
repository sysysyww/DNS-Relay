#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct task
{
    void (*run)(void *arg); // function pointer
    void *arg;              // function parameter
    struct task *next;      // next task
} task;
// 线程池
typedef struct threadpool
{
    task *first;    // head
    task *end;      // tail
    int thread_num; // thread number
    int task_size;  // task number
    pthread_mutex_t mute_pool;
    pthread_cond_t not_empty;
    int shut_down; // 1 destroy 0 not destroy
} threadpool;

void *worker(void *arg)
{

    threadpool *pool = (threadpool *)arg;
    while (1)
    {
        pthread_mutex_lock(&pool->mute_pool);
        // task is empty
        while (pool->first == NULL && !pool->shut_down)
        {
            // stop thread
            pthread_cond_wait(&pool->not_empty, &pool->mute_pool);
        }
        // check thread pool is shut down or not
        if (pool->shut_down)
        {
            pthread_mutex_unlock(&pool->mute_pool);
            pthread_exit(NULL);
        }
        // get task
        task *t = pool->first;
        pool->first = t->next;
        pool->task_size--;
        pthread_mutex_unlock(&pool->mute_pool);
        t->run(t->arg);
        free(t);
        t = NULL;
    }
    return NULL;
}

int thread_pool_init(int number, threadpool *pool)
{
    pool = (threadpool *)malloc(sizeof(threadpool));
    if (pool == NULL)
    {
        printf("create thread pool error\n");
        return 0;
    }

    // (*pool)->thread_num = number;
    // (*pool)->task_size = 0;
    // (*pool)->first = NULL;
    // (*pool)->end = NULL;

    pool->thread_num = number;
    pool->task_size = 0;
    pool->first = NULL;
    pool->end = NULL;

    pthread_mutex_init(&pool->mute_pool, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pool->shut_down = 0;
    // create thread
    for (int i = 0; i < number; i++)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, worker, pool);
    }

    return 1;
}

threadpool *thread_pool_initial(int number)
{
    threadpool *pool = (threadpool *)malloc(sizeof(threadpool));
    if (pool == NULL)
    {
        printf("malloc error\n");
        return NULL;
    }
    pool->thread_num = number;
    pool->task_size = 0;
    pool->first = NULL;
    pool->end = NULL;
    // 锁和条件变量初化
    pthread_mutex_init(&pool->mute_pool, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pool->shut_down = 0;
    // 创建线程
    for (int i = 0; i < number; i++)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, worker, pool);
    }
    return pool;
}

int thread_pool_add(threadpool *pool, void (*run)(void *), void *arg)
{
    if (pool->shut_down) // 线程池已经被关闭
    {
        return 0;
    }
    task *t = (task *)malloc(sizeof(task));
    t->arg = arg;
    t->run = run;
    t->next = NULL;
    pthread_mutex_lock(&pool->mute_pool);
    if (pool->first == NULL) // 第一个任务
    {
        pool->first = t;
        pool->end = t;
    }
    else
    {
        pool->end->next = t;
        pool->end = t;
    }
    pool->task_size++;
    pthread_cond_signal(&pool->not_empty); // 唤醒阻塞的线程
    pthread_mutex_unlock(&pool->mute_pool);
    return 1;
}

int thread_pool_destroy(threadpool *pool)
{
    if (pool == NULL)
    {
        return -1;
    }
    pool->shut_down = 1; // 关闭线程池
    // 唤醒阻塞的消费者线程顺便销毁
    for (int i = 0; i < pool->thread_num; i++)
    {
        pthread_cond_signal(&pool->not_empty);
    }
    // 回收锁和条件变量
    pthread_mutex_destroy(&pool->mute_pool);
    pthread_cond_destroy(&pool->not_empty);
    // 释放线程池
    free(pool);
    pool = NULL;
    return 0;
}
