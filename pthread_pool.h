#ifndef PTHREAD_POOL_H
#define PTHREAD_POOL_H

#include <pthread.h>

// Task structure
typedef struct task
{
    void (*run)(void *arg); // function pointer
    void *arg;              // function parameter
    struct task *next;      // next task
} task;

// Thread pool structure
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

// Function prototypes
void *worker(void *arg);
int thread_pool_init(int number, threadpool *pool);
threadpool *thread_pool_initial(int number);
int thread_pool_add(threadpool *pool, void (*run)(void *), void *arg);
int thread_pool_destroy(threadpool *pool);

#endif // PTHREAD_POOL_H
