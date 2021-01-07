#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ThreadRoutines.h"

// Default Thread priority is 1
static int turn = 1;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition2 = PTHREAD_COND_INITIALIZER;

void *ThreadRoutine1(void *arg) {

  SharedBlock *sharedMemory = (SharedBlock *)arg;
  HashTable *table = createHashTable(CAPACITY);
  Page *page;

  // File to read
  FILE *gccFile;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  // Other variables
  unsigned int traceAddress;
  char traceType;
  int contiguousEntries = 0;
  int pageReference = 0;

  if ((gccFile = fopen("gcc.trace", "r")) == NULL) {
    printf("Couldn't open gcc.trace.\n");
    return (int *)1;
  }

  while (sharedMemory->iterations < sharedMemory->maxTraceEntries) {

    pthread_mutex_lock(&lock);
    while (turn != 1) pthread_cond_wait(&condition1, &lock);

    for (contiguousEntries = 0; contiguousEntries < sharedMemory->q; contiguousEntries++) {

      if (sharedMemory->iterations >= sharedMemory->maxTraceEntries) break;

      if ((read = getline(&line, &len, gccFile)) != -1) {

        sscanf(line, "%x %c", &traceAddress, &traceType);
        page = createPage(traceAddress, traceType, PID_1);

          // Check if the page is in queue
        if (!sharedMemory->chosenAlgorithm){
          // Case LRU
          pageReference = lruReferToPageInQueue(sharedMemory->queue, table, page);
          if (!pageReference) sharedMemory->pageFaults++;
          else if (pageReference == 1) sharedMemory->hits++;
          else sharedMemory->updates++;
        } else {
          // Case second chance
          pageReference = secondChanceReferToPageInQueue(sharedMemory->queue, table, page);
          if (!pageReference) sharedMemory->pageFaults++;
          else if (pageReference == 1) sharedMemory->hits++;
          else sharedMemory->updates++;
        }

        // Increment Read/Write counter depending on the trace type
        if (page->traceType == 'R') sharedMemory->numOfReads++;
        else sharedMemory->numOfWrites++;

        // Increase the counter of the current process trace reads (gcc.trace)
        sharedMemory->gccTraceBits++;

        sharedMemory->iterations++;
      }
    }

    // Notify Thread 2
    turn = 2;
    pthread_cond_signal(&condition2);
    pthread_mutex_unlock(&lock);

  }

  pthread_mutex_lock(&lock);
  printf("\nHash Table 1:");
  printHashTable(table);
  printf("Occupied pages in table 1: %d\n", table->totalPages);
  pthread_mutex_unlock(&lock);

  deleteHashTable(table);
  free(page);
  fclose(gccFile);

  return NULL;
}

void *ThreadRoutine2(void *arg) {

  SharedBlock *sharedMemory = (SharedBlock *)arg;
  HashTable *table = createHashTable(CAPACITY);
  Page *page;

  // File to read
  FILE *bzipFile;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  // Other variables
  unsigned int traceAddress;
  char traceType;
  int contiguousEntries = 0;
  int pageReference = 0;

  if ((bzipFile = fopen("bzip.trace", "r")) == NULL) {
    printf("Couldn't open bzip.trace.\n");
    return (int *)1;
  }

  while (sharedMemory->iterations < sharedMemory->maxTraceEntries) {

    pthread_mutex_lock(&lock);
    while (turn != 2) pthread_cond_wait(&condition2, &lock);

    for (contiguousEntries = 0; contiguousEntries < sharedMemory->q; contiguousEntries++) {

      if (sharedMemory->iterations >= sharedMemory->maxTraceEntries) break;

      if ((read = getline(&line, &len, bzipFile)) != -1) {

        sscanf(line, "%x %c", &traceAddress, &traceType);
        page = createPage(traceAddress, traceType, PID_2);

        // Check if the page is in queue
        if (!sharedMemory->chosenAlgorithm) {
          // Case LRU
          pageReference = lruReferToPageInQueue(sharedMemory->queue, table, page);
          if (!pageReference) sharedMemory->pageFaults++;
          else if (pageReference == 1) sharedMemory->hits++;
          else sharedMemory->updates++;
        } else {
          // Case second chance
          pageReference = secondChanceReferToPageInQueue(sharedMemory->queue, table, page);
          if (!pageReference) sharedMemory->pageFaults++;
          else if (pageReference == 1) sharedMemory->hits++;
          else sharedMemory->updates++;
        }

        // Increment Read/Write counter depending on the trace type
        if (page->traceType == 'R') sharedMemory->numOfReads++;
        else sharedMemory->numOfWrites++;

        // Increase the counter of the current process trace reads (bzip.trace)
        sharedMemory->bzipTraceBits++;

        sharedMemory->iterations++;
      }
    }

    // Notify Thread 1
    turn = 1;
    pthread_cond_signal(&condition1);
    pthread_mutex_unlock(&lock);

  }

  pthread_mutex_lock(&lock);
  printf("\nHash Table 2:");
  printHashTable(table);
  printf("Occupied pages in table 2: %d\n", table->totalPages);
  pthread_mutex_unlock(&lock);

  deleteHashTable(table);
  free(page);
  fclose(bzipFile);

  return NULL;
}