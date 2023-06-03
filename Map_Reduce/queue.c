#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void initialize(queue_t* queue) {
    queue->front = NULL;
    queue->rear = NULL;

    sem_init(&queue->sem, 0, 0);
    pthread_mutex_init(&queue->mutex, NULL);
}

int isEmpty(queue_t* queue) {
    return queue->front == NULL;
}

void enqueue(queue_t* queue, char* data) {
    node_t* new_node = malloc(sizeof(node_t));
    new_node->data = data;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (isEmpty(queue)) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->sem);
}

void dequeue(queue_t* queue, char** data) {
    sem_wait(&queue->sem);
    if (isEmpty(queue)) {
        *data = NULL;
        return;
    }

    pthread_mutex_lock(&queue->mutex);
    node_t* temp = queue->front;
    *data = temp->data;

    if (queue->front == queue->rear) {
        queue->front = NULL;
        queue->rear = NULL;
    } else {
        queue->front = queue->front->next;
    }
    pthread_mutex_unlock(&queue->mutex);
}