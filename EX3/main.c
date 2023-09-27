
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include "unbounded_queue.h"
#include "bounded_queue.h"
#define NUM_OF_CONSUMERS 3
#define SCREEN_MANAGER 1000
#define SPORT_QUEUE 0
#define WEATHER_QUEUE 1
#define POLITICS_QUEUE 2

// create a global array of bounded queues
boundedQueue **producers;
unboundedQueue *consumers[NUM_OF_CONSUMERS];
boundedQueue *screenMonitorQueue;

void printType(newsItem item) {
    switch (item.type) {
        case sports:
            printf("SPORTS");
            break;
        case weather:
            printf("WEATHER");
            break;
        case news:
            printf("NEWS");
            break;
        default:
            printf("error\n");
            break;
    }
}

// generate a random newsType
int generateRandomNewsType() {
    // srand  by the tid
    // generate a random number between 0 and 2
    int random = rand() % 3;
    // return the random number
    return random;
}

//create a news item
newsItem createNewsItem(int i) {
    // create a newsItem
    newsItem item;
    // allocate memory for the id
    item.id = i;
    // set the id of the newsItem to random
    // set the type of the newsItem
    item.type = generateRandomNewsType();
    // get the number of the type of newsItem
    if (i != -1) {
        item.numOfType = producers[i]->types[item.type];
        producers[i]->types[item.type]++;
    } else {
        item.numOfType = 0;
    }
    // return the newsItem
    return item;
}


void sortAndAdd(newsItem item);


void printNewsItem(newsItem item, int i);


void freeAll(int numOfProducers);

// add bounded queue to the global array of bounded queues
void addBoundedQueue(boundedQueue *queue, int numOfProducers) {
    // realloc the global array of bounded queues
    producers = realloc(producers, sizeof(boundedQueue) * (numOfProducers + 1));
    // add the queue to the global array of bounded queues
    producers[numOfProducers - 1] = queue;
}

// create insertBounded function to boundedbuffer
void insertBounded(boundedQueue *queue, newsItem item) {
    // down the empty semaphore
    sem_wait(&queue->empty);
    // lock the mutex of the queue
    pthread_mutex_lock(&queue->mutex);

    // insert the newsItem into the queue
    queue->buffer[queue->rear] = item;
    // increment the number of the type of newsItem
    // increment the rear of the queue
    queue->rear = (queue->rear + 1) % queue->capacity;
    // increment the count of the queue
    queue->count++;
    // unlock the mutex
    pthread_mutex_unlock(&queue->mutex);
    // post to the full semaphore
    sem_post(&queue->full);
}

// create a bounded pop function to popBounded newsItems off of the queue
newsItem popBounded(boundedQueue *queue) {
    // print num of items left
    // down the full semapore
    sem_wait(&queue->full);

    // lock the mutex of the queue
    pthread_mutex_lock(&queue->mutex);
    // popBounded the newsItem off of the queue
    newsItem item = queue->buffer[queue->front];
    // increment the front of the queue
    queue->front = (queue->front + 1) % queue->capacity;
    // decrement the count of the queue
    queue->count--;
    // unlock the mutex
    pthread_mutex_unlock(&queue->mutex);
    // post to the empty semaphore
    sem_post(&queue->empty);
    // return the newsItem
    return item;
}

// Insert a newsItem into the unbounded queue
void insertUnbounded(unboundedQueue *queue, newsItem item) {
    // Create a new node for the item
    Node *newNode = (Node *) malloc(sizeof(Node));
    newNode->item = item;
    newNode->next = NULL;

    // Lock the mutex of the queue
    pthread_mutex_lock(&queue->mutex);

    // Insert the item at the tail of the queue
    if (queue->tail == NULL) {
        // If the queue is empty, update both head and tail pointers
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        // Otherwise, update the tail pointer
        queue->tail->next = newNode;
        queue->tail = newNode;
    }

    // Increment the count of the queue
    queue->count++;

    // Unlock the mutex
    pthread_mutex_unlock(&queue->mutex);

    // Post to the full semaphore
    sem_post(&queue->full);
}

// Pop a newsItem from the unbounded queue
newsItem popUnbounded(unboundedQueue *queue) {
    // Down the full semaphore
    sem_wait(&queue->full);
    // Lock the mutex of the queue
    pthread_mutex_lock(&queue->mutex);

    if (queue->head == NULL) {
        // Unlock the mutex before returning
        pthread_mutex_unlock(&queue->mutex);
        return createNewsItem(-1); // or handle the empty queue case appropriately
    }
    // Get the item from the head of the queue
    Node *nodeToRemove = queue->head;
    newsItem item = nodeToRemove->item;

    // Update the head pointer
    queue->head = queue->head->next;

    // If the head becomes NULL, update the tail pointer as well
    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    // Decrement the count of the queue
    queue->count--;

    // Free the memory for the removed node
    free(nodeToRemove);

    // Unlock the mutex
    pthread_mutex_unlock(&queue->mutex);

    return item;
}

// create a producer function
void *producer(void *args) {
    int* ptr = ((int*) args);
    int indexOfQueue = *ptr;
    // create a counter to keep track of the number of newsItems produced

    for (int i = 0; i < producers[indexOfQueue]->numOfProducts; i++) {
        newsItem item = createNewsItem(indexOfQueue);
        // insertBounded the newsItem into the queue
        insertBounded(producers[indexOfQueue], item);
        // increment the number of the type of newsItem
    }
    // set the finished flag to -1
    producers[indexOfQueue]->finished = -1;
    return NULL;
}


void *dispatcher(void *args) {
    int numOfProducers = *(int*) args;
    int totalProducts = 0;
    // calaculate the total number of products
    for (int i = 0; i < numOfProducers; i++) {
        totalProducts += producers[i]->numOfProducts;
    }
    // the dispatcher will go through the global array of bounded queues and popBounded the newsItems off of the queue in round robin fashion
    // create a counter to keep track of the index of the queue
    int counter = 0;
    // create a newsItem to hold the newsItem that is popped off of the queue
    // a counter to check if all of the newsItems have been popped off of the queues
    int isAllFinished = 0;
    // create a while loop that will run until all of the newsItems have been popped off of the queues

    // arr of flags to check if the producers are finished
    int *finishedFlags = malloc(sizeof(int) * numOfProducers);
    // set all of the flags to 0
    for (int i = 0; i < numOfProducers; i++) {
        finishedFlags[i] = 0;
    }
    int currNumOfNewsItems = 0;
    while (1) {
        for (counter = 0; counter < numOfProducers; counter++) {
            // check if finished
            if (producers[counter]->finished == -1) {
                finishedFlags[counter] = 1;
                // check if the queue is empty and if finished
                if (producers[counter]->count == 0) {
                    continue;
                }
            }
            // popBounded the last newsItem off of the queue
            newsItem item = popBounded(producers[counter]);
            sortAndAdd(item);
            // increment the counter
            currNumOfNewsItems++;
        }
        // check if all of the producers are finished
        for (int i = 0; i < numOfProducers; i++) {
            if (finishedFlags[i] == 0) {
                isAllFinished = 0;
                break;
            }
            isAllFinished = 1;
        }
        // if all of the producers are finished, break out of the loop
        if (isAllFinished == 1 && currNumOfNewsItems == totalProducts) {
            // set the finished flag to -1
            for (int i = 0; i < 3; i++) {
                consumers[i]->finished = -1;
            }
            break;
        }
    }
    free(finishedFlags);
    return NULL;
}

void sortAndAdd(newsItem item) {
    switch (item.type) {
        case sports:
            insertUnbounded(consumers[SPORT_QUEUE], item);
            break;
        case weather:
            insertUnbounded(consumers[WEATHER_QUEUE], item);
            break;
        case news:
            insertUnbounded(consumers[POLITICS_QUEUE], item);
            break;
    }
}

void *coEditor(void *args) {
    int indexOfQueue = *(int*) args;
    // create a counter to keep track of the number of newsItems produced
    while (1) {
        // check if the queue is empty
        if (consumers[indexOfQueue]->count == 0) {
            // if it is empty, check if the finished flag is set to -1
            if (consumers[indexOfQueue]->finished == -1) {
                break;
            }
            continue;
        }

        // popBounded the last newsItem off of the queue
        newsItem item = popUnbounded(consumers[indexOfQueue]);
        // sleep for a  millisecond
        usleep(100000);
        insertBounded(screenMonitorQueue, item);
    }
    // mutex
    pthread_mutex_lock(&screenMonitorQueue->mutex);
    // set the finished flag to -1
    screenMonitorQueue->finished += 1;
    // unlock the mutex
    pthread_mutex_unlock(&screenMonitorQueue->mutex);

    return NULL;
}

void *screenMonitor(void *args) {
    int numOfItems = 0;
    // create a counter to keep track of the number of coeditors that are finished
    while (1) {
        if (screenMonitorQueue->count == 0) {
            // check if the queue is finished
            if (screenMonitorQueue->finished == 3) {
                printf("DONE");
                break;
            }
            continue;
        }
        // popBounded the last newsItem off of the queue
        newsItem item = popBounded(screenMonitorQueue);
        // increment the number of the type of newsItem
        // print the newsItem
        numOfItems++;
        fflush(stdout);
        printNewsItem(item, numOfItems);
    }
    return NULL;
}

void printNewsItem(newsItem item, int i) {
    printf("PRODUCER %d ", item.id + 1);
    printType(item);
    printf(" %d\n", item.numOfType + 1);

}

// create a bounded queue
boundedQueue *createBoundedQueue(int id, int size, int numOfProducts) {
    // create a bounded queue
    boundedQueue *queue = malloc(sizeof(boundedQueue));
    // set the id of the queue
    queue->id = id;
    // set the size of the queue
    queue->capacity = size;
    // set the number of products of the queue
    queue->numOfProducts = numOfProducts;
    // set the front of the queue
    queue->front = 0;
    // set the rear of the queue
    queue->rear = 0;
    // set the count of the queue
    queue->count = 0;
    // set the finished flag to 0
    queue->finished = 0;
    // initialize the mutex
    pthread_mutex_init(&queue->mutex, NULL);
    // initialize the full semaphore
    sem_init(&queue->full, 0, 0);
    // initialize the empty semaphore
    sem_init(&queue->empty, 0, size);
    // allocate memory for the buffer
    queue->buffer = malloc(sizeof(newsItem) * (size + 1));

    //queue->types = malloc(3 * sizeof(int));
    queue->types[0] = 0;
    queue->types[1] = 0;
    queue->types[2] = 0;
    return queue;
}

// Initialize the unbounded queue
void initUnbounded(unboundedQueue *queue) {
    queue->id = 0;
    queue->tail = NULL;
    queue->head = NULL;
    queue->count = 0;
    queue->finished = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    sem_init(&queue->full, 0, 0);
}

// Create an unbounded queue
unboundedQueue *createUnboundedQueue(int id) {
    // Create a new unbounded queue
    unboundedQueue *queue = (unboundedQueue *) malloc(sizeof(unboundedQueue));

    // Set the id of the queue
    queue->id = id;

    // Initialize the queue
    initUnbounded(queue);

    return queue;
}

// create the consumers unbounded queues
void createConsumersQueues() {
    // create a consumer for each consumer
    for (int i = 0; i < NUM_OF_CONSUMERS; i++) {
        // create a consumer
        consumers[i] = createUnboundedQueue(i);
    }

}

int main(int argc, char *argv[]) {

    int numOfProducers = 0;
    int coEditorQueueSize = 0;

    // the first arg is the path to the config file
    // the first arg is the path to the config file
    FILE *configFile = fopen(argv[1], "r");

    if (configFile == NULL) {
        printf("Failed to open the config file.\n");
        return 1;
    }
    int producerId, numProducts, boundedQueueSize;

    while (fscanf(configFile, "%d%d%d", &producerId, &numProducts, &boundedQueueSize) == 3) {
        numOfProducers++;
        // create a bounded queue
        // add the bounded queue to the global array of bounded queues
        addBoundedQueue(createBoundedQueue(producerId, boundedQueueSize, numProducts), numOfProducers);
    }
    // Close the config file
    fclose(configFile);
    // last line of the config file is the size of the co editors queue
    coEditorQueueSize = producerId;
    // create the screen monitor queue for the co editors
    screenMonitorQueue = createBoundedQueue(SCREEN_MANAGER, coEditorQueueSize, 0);
    // create the consumers queues
    createConsumersQueues();

    // create the producer threads
    pthread_t *producerTid = malloc(sizeof(pthread_t) * numOfProducers);
    // build the bounded queues
    int j = 0;
    int queueIndices[numOfProducers]; // Create an array to hold the indices

    for (; j < numOfProducers; j++) {
        queueIndices[j] = j; // Assign the current index value to the array
        if (pthread_create(&producerTid[j], NULL, producer, (void *)&queueIndices[j]) != 0) {
            perror("failed to create producer thread");
        }
    }
    // create the dispatcher thread
    pthread_t dispatcherTid;
    if (pthread_create(&dispatcherTid, NULL, dispatcher, (void *) &numOfProducers) != 0) {
        perror("failed to create dispatcher thread");
    }
    // there are 3 consumers : sports , news , weather
    pthread_t newsUnboundedTid[NUM_OF_CONSUMERS];
    int consumerIds[NUM_OF_CONSUMERS] = {0, 1, 2};
    // build the unbounded queues for the co editors
    for (int i = 0; i < NUM_OF_CONSUMERS; i++) {
        if (pthread_create(&newsUnboundedTid[i], NULL, coEditor, (void *) &consumerIds[i]) != 0) {
            perror("failed to create consumer thread");
        }
    }
    pthread_t monitorTid;
    if (pthread_create(&monitorTid, NULL, screenMonitor, NULL) != 0) {
        perror("failed to create monitor thread");
    }
    // join the producer threads
    for (int i = 0; i < numOfProducers; i++) {
        if (pthread_join(producerTid[i], NULL) != 0) {
            perror("failed to join producer thread tid");
        }
    }
    // join the dispatcher thread
    if (pthread_join(dispatcherTid, NULL) != 0) {
        perror("failed to join dispatcher thread tid");
    }
    // join the consumer threads
    for (int i = 0; i < NUM_OF_CONSUMERS; i++) {
        if (pthread_join(newsUnboundedTid[i], NULL) != 0) {
            perror("failed to join consumer thread tid");
        }
    }
    // join the monitor thread
    if (pthread_join(monitorTid, NULL) != 0) {
        perror("failed to join monitor thread tid");
    }
    freeAll(numOfProducers);
    free(producerTid);
    return 0;
}

void freeBoundedQueue(boundedQueue *queue) {
    //free(queue->types);
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->full);
    sem_destroy(&queue->empty);
    free(queue);
}
void freeUnboundedQueue(unboundedQueue *queue) {
    Node *current = queue->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    pthread_mutex_destroy(&queue->mutex);
    sem_destroy(&queue->full);
    free(queue);
}
void freeAll(int numOfProducers) {
    for (int i = 0; i < numOfProducers; i++) {
        freeBoundedQueue(producers[i]);
    }
    free(producers);
    for (int i = 0; i < NUM_OF_CONSUMERS; i++) {
        freeUnboundedQueue(consumers[i]);
    }
    free(screenMonitorQueue);
}


