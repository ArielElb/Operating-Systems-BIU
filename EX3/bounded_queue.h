#ifndef BOUNDED_QUEUE_H
#define BOUNDED_QUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include "newsItem.h"

// create a bounded queue to hold newsItems
typedef struct {
    // 0 for sports, 1 for weather, 2 for news
    int types[3];
    int numOfProducts;
    int id;
    newsItem *buffer;
    // capacity of the queue
    int capacity;
    // front and rear of the queue
    int front;
    // rear of the queue
    int rear;
    // count of the number of items in the queue
    int count;
    // flag to check if finished producing
    int finished;
    // mutex, full, and empty semaphores
    pthread_mutex_t mutex;
    sem_t full;
    sem_t empty;
} boundedQueue;

void insertBounded(boundedQueue *queue, newsItem item);
void addBoundedQueue(boundedQueue *queue, int numOfProducers);
void insertBounded(boundedQueue *queue, newsItem item);
newsItem popBounded(boundedQueue *queue);
boundedQueue *createBoundedQueue(int id, int size, int numOfProducts);



#endif