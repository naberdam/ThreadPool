//
// Created by nerya on 21/06/2020.
//
#ifndef __OS_QUEUE__
#define __OS_QUEUE__


typedef struct os_node
{
	struct os_node* next;
	void* data;
}OSNode;

typedef struct os_queue
{
	OSNode *head, *tail;

}OSQueue;

OSQueue* osCreateQueue();

void osDestroyQueue(OSQueue* queue);

int osIsQueueEmpty(OSQueue* queue);

void osEnqueue(OSQueue* queue, void* data);

void* osDequeue(OSQueue* queue);


#endif
