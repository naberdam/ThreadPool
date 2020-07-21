// Nerya Aberdam 311416457 LATE-SUBMISSION
#include "threadPool.h"
#include <stdlib.h>
#define FAILURE -1
#define SUCCEED 0
#define SYSTEM_CALL_ERR "ERROR in System Call\n"
#define PTHREAD_ERR "error with pthread function\n"
// Function to write an error
void myPerror(const char* msg)
{
	perror(msg);
}
// Function that checks if the first given variable is not equal to zero and the write an error
void errorOfZero(int checkIfZero, const char* msg)
{
	if (checkIfZero != 0) {
		myPerror(msg);
	}
}
// Function that write an error in case that we need to exit from program
void myPerrorAndFreeMemoryWithExit(ThreadPool* threadPool, const char* msg)
{
	myPerror(msg);
	exit(-1);
}
// Function that checks if first given variable is NULL, and if it does so it calls
// to write and error and exit from program
void errorWithNull(void* checkIfNull, ThreadPool* threadPool, const char* msg)
{
	if (checkIfNull == NULL) {
		free(threadPool);
		myPerrorAndFreeMemoryWithExit(threadPool, msg);
	}
}
// Function that checks if first given variable is not equal to zero, and if it does so it calls
// to write and error and exit from program
void errorAndFreeMemoryOfZero(int checkIfZero, ThreadPool* threadPool, const char* msg)
{
	if (checkIfZero != 0) {
		myPerrorAndFreeMemoryWithExit(threadPool, msg);
	}
}
// Function that frees task
void taskFree(Task* task)
{
	if (task == NULL) {
		return;
	}
	(task->compFunc)(task->params);
	free(task);
}
// Function that does while loop to free all tasks
void dequeueFromOs(OSQueue* tasks)
{
	while (!osIsQueueEmpty(tasks)) {
		Task* t = osDequeue(tasks);
		free(t);
	}
}
// Function that checks the availability of run in runThread function
int checkAvailabilityOfRunInExecute(ThreadPool* threadPool) {
	if (osIsQueueEmpty(threadPool->queue) && threadPool->canNotRun == 0) {
		errorAndFreeMemoryOfZero(pthread_cond_wait(&(threadPool->cond), &(threadPool->lock)),
			threadPool,
			PTHREAD_ERR);
	}
	if (osIsQueueEmpty(threadPool->queue) && threadPool->canNotRun == 1
		&& threadPool->canRun == 1) {
		return FAILURE;
	}
	return SUCCEED;
}
// Function that runs the threads
void* runThread(void* threadPool)
{
	// make cast for the threadPool
	ThreadPool* threadPool1 = (ThreadPool*)threadPool;
	Task* task;
	// loop until that we can run the tasks for the threadPool
	while (threadPool1->canRun == 1) {
		errorAndFreeMemoryOfZero(pthread_mutex_lock(&(threadPool1->lock)), threadPool1, PTHREAD_ERR);
		if (checkAvailabilityOfRunInExecute(threadPool1) == FAILURE) {
			break;
		}
		// get the task out.
		task = (Task*)osDequeue(threadPool1->queue);
		errorOfZero(pthread_mutex_unlock(&(threadPool1->lock)), PTHREAD_ERR);
		taskFree(task);
	}
	errorAndFreeMemoryOfZero(pthread_mutex_unlock(&(threadPool1->lock)), threadPool1, PTHREAD_ERR);
	return NULL;
}
// Function that free tasks and destroy mutex and cond
void freeTasksAndDestroyMutexCond(OSQueue* queue, pthread_t* threads, pthread_mutex_t lock, pthread_cond_t cond)
{
	// free all what allocate.
	dequeueFromOs(queue);
	osDestroyQueue(queue);
	if (threads != NULL) { free(threads); }
	pthread_mutex_destroy(&(lock));
	pthread_cond_destroy(&(cond));
}
// Function that is called in the end of tpDestroy (does not matter if there is problem or not)
void endOfTpDestroy(ThreadPool* threadPool)
{
	freeTasksAndDestroyMutexCond(threadPool->queue, threadPool->threads, threadPool->lock, threadPool->cond);
	free(threadPool);
}
// Function that does action on thread - join or create
void actionOfEachThread(ThreadPool* threadPool, int create)
{
	int i;
	if (threadPool->threads != NULL) {
		for (i = 0; i < threadPool->numOfThreads; i++) {
			if (!create) {
				errorAndFreeMemoryOfZero(pthread_join(threadPool->threads[i], NULL),
					threadPool, PTHREAD_ERR);
			} else {
				errorOfZero(pthread_create(&(threadPool->threads[i]), NULL,
					runThread, (void*)threadPool), PTHREAD_ERR);
			}
		}
	}
}
// Function that destroys process
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks)
{
	// case we got a NULL in threadPool
	if (threadPool == NULL) { return; }
	threadPool->canRun = shouldWaitForTasks;
	if (threadPool->canNotRun == 0) {
		errorOfZero(pthread_mutex_lock(&threadPool->lock), PTHREAD_ERR);
		threadPool->canNotRun = 1;
		int pthreadCondBroadcast, pthreadMutexUnlock;
		errorOfZero((pthreadCondBroadcast = pthread_cond_broadcast(&(threadPool->cond))), PTHREAD_ERR);
		errorOfZero((pthreadMutexUnlock = pthread_mutex_unlock(&(threadPool->lock))), PTHREAD_ERR);
		if ((pthreadCondBroadcast != SUCCEED) || pthreadMutexUnlock != SUCCEED) {
			endOfTpDestroy(threadPool);
			return;
		}
		actionOfEachThread(threadPool, 0);
	}
	endOfTpDestroy(threadPool);
}
// Function that insert a task
int tpInsertTask(ThreadPool* threadPool, void (* compFunc)(void*), void* param)
{
	// error that threadPool is NULL or we can't run the threadPool (mean threads) or there is an error in function
	if (threadPool == NULL || threadPool->canNotRun == 1 || compFunc == NULL) { return FAILURE; }
	Task* task;
	// allocating task with malloc and check if the allocation succeed or not
	errorWithNull((task = (Task*)malloc(sizeof(Task))), threadPool, PTHREAD_ERR);
	task->compFunc = compFunc;
	task->params = param;
	// lock the mutex to prevent intersection and check it
	errorAndFreeMemoryOfZero(pthread_mutex_lock(&(threadPool->lock)), threadPool, PTHREAD_ERR);
	osEnqueue(threadPool->queue, (void*)task);
	errorAndFreeMemoryOfZero(pthread_cond_signal(&(threadPool->cond)), threadPool, PTHREAD_ERR);
	errorAndFreeMemoryOfZero(pthread_mutex_unlock(&threadPool->lock), threadPool, PTHREAD_ERR);
	// everything was good so return SUCCEED
	return SUCCEED;
}
// Function that creates threads according to given numOfThreads
ThreadPool* tpCreate(int numOfThreads)
{
	// case we got an negative numOfThreads.
	if (numOfThreads <= 0) { return NULL; }
	// allocation for the threadPool
	ThreadPool* threadPool = (ThreadPool*)malloc(sizeof(ThreadPool));
	// case that the allocation failed.
	if (threadPool == NULL) { myPerrorAndFreeMemoryWithExit(threadPool, SYSTEM_CALL_ERR); }
	// make an array of threads according the number of thread we got.
	threadPool->threads = (pthread_t*)malloc(sizeof(pthread_t) * numOfThreads);
	if (threadPool->threads == NULL) { myPerrorAndFreeMemoryWithExit(threadPool, SYSTEM_CALL_ERR); }
	errorOfZero(pthread_mutex_init(&(threadPool->lock), NULL), PTHREAD_ERR);
	threadPool->queue = osCreateQueue();
	// make a boolean that indicate we can run the threads and cant run the threads
	threadPool->canRun = 1;
	threadPool->canNotRun = 0;
	errorOfZero(pthread_cond_init(&(threadPool->cond), NULL), PTHREAD_ERR);
	threadPool->numOfThreads = numOfThreads;
	// execute threads
	actionOfEachThread(threadPool, 1);
	return threadPool;
}