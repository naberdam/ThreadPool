//
// Created by nerya on 21/06/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "osqueue.h"
#include "threadPool.h"
#define NUMBER_OF_THREADS 2
#define NUMBER_OF_TASKS 10
#define NUMBER_OF_TIMES 100

int countHello = 0;
int countBye = 0;

void hello (void* a)
{
	countHello++;
}
void bye(void* a) {

	countBye++;
}

void test_thread_pool_sanity()
{
	int i;

	ThreadPool* tpFinishAllTasks = tpCreate(NUMBER_OF_THREADS);
	ThreadPool* tpNotFinishAllTasks = tpCreate(NUMBER_OF_THREADS);

	for(i = 0; i < NUMBER_OF_TASKS; ++i)
	{
		tpInsertTask(tpNotFinishAllTasks,bye,NULL);
		tpInsertTask(tpFinishAllTasks,hello,NULL);

	}
	tpDestroy(tpNotFinishAllTasks,0);
	tpDestroy(tpFinishAllTasks,1);

}

int main()
{
	int i;
	for (int j = 0; j < NUMBER_OF_TIMES; ++j) {
		for (i = 0; i < NUMBER_OF_TIMES; i++) {
			test_thread_pool_sanity();
			printf("I am not finishing all tasks. number of tasks finished:  %d\n", countBye);
			countBye = 0;
			printf("I need to finish all tasks. number of tasks finished:  %d\n", countHello);
			countHello = 0;
		}
	}


	return 0;
}