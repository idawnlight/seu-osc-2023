#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

typedef struct node {
    char* data;
    struct node* next;
} node_t;

typedef struct queue {
    node_t* front;
    node_t* rear;

    sem_t sem;
    pthread_mutex_t mutex;
} queue_t;

void initialize(queue_t* queue);
int isEmpty(queue_t* queue);
void enqueue(queue_t* queue, char* data);
void dequeue(queue_t* queue, char** data);

#endif // QUEUE_H
