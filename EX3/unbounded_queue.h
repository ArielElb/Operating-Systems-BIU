#ifndef UNBOUNDED_QUEUE_H
#define UNBOUNDED_QUEUE_H

#include "newsItem.h"

// Node structure for the linked list
typedef struct Node {
    newsItem item;
    struct Node *next;
} Node;
// Create an unbounded queue to hold newsItems
typedef struct {
    int id;
    Node *head; // Updated variable name
    Node *tail; // Updated variable name
    int count;
    int finished;
    pthread_mutex_t mutex;
    sem_t full;
} unboundedQueue;

void insertUnbounded(unboundedQueue *queue, newsItem item);

newsItem popUnbounded(unboundedQueue *queue);

unboundedQueue *createUnboundedQueue(int id);

void initUnbounded(unboundedQueue *queue);


#endif
