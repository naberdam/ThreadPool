// Nerya Aberdam 311416457 LATE-SUBMISSION
#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>
#include "osqueue.h"


typedef struct thread_pool
{
	int numOfThreads;
	pthread_t* threads;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	bool canNotRun;
	bool canRun;
	OSQueue* queue;

}ThreadPool;
typedef struct {
	void (*compFunc)(void *);
	void *params;
} Task;
ThreadPool* tpCreate(int numOfThreads);
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);
int tpInsertTask(ThreadPool* threadPool, void (*compFunc) (void *), void* param);

#endif