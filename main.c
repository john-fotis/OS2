#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SharedMemory.h"
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>

#include "SimulationLibrary.h"

#define PROCESS1_ID 1
#define PROCESS2_ID 2

int main (int argc, char * argv[]) {

  // Check input validity
  if (!(argc == 4 || argc == 5)) {
    printf("Correct usage:\n \
    ./main [LRU->0 | SECOND_CHANCE->1] \
    [RAM frames number] \
    [Sequential character for each file (q)] \
    (Max number of trace characters to read) \n");
    return 1;
  }

  // Get execution arguments
  int chosenAlgorithm = atoi(argv[1]);
  int framesNumber = atoi(argv[2]);
  int q = atoi(argv[3]);
  int maxTraceBits = RAND_MAX;
  if (argc == 5)
    maxTraceBits = atoi(argv[4]);

  // Set up semaphores
  sem_t *mutex;
  sem_t *sem_p1;
  sem_t *sem_p2;

  sem_unlink(MUTEX);
  sem_unlink(SEM_P1);
  sem_unlink(SEM_P2);

  // ======= Mutex semaphore =======
  if ((mutex = sem_open(MUTEX, O_CREAT, 0660, 1)) == SEM_FAILED)
  {
    printf("sem_open/mutex\n");
    return -1;
  }

  // ======= P1 semaphore =======
  if ((sem_p1 = sem_open(SEM_P1, O_CREAT, 0660, 1)) == SEM_FAILED)
  {
    printf("sem_open/p1\n");
    return -1;
  }

  // ======= P2 semaphore =======
  if ((sem_p2 = sem_open(SEM_P2, O_CREAT, 0660, 0)) == SEM_FAILED)
  {
    printf("sem_open/p2\n");
    return -1;
  }

  int maxQueueMemory = sizeof(Page) * framesNumber;

  // ======= Get the shared memory block =======
  Block *sharedMemory = attachBlock(FILENAME, sizeof(Block) + maxQueueMemory, SHARED_MEMORY_ID);
  if(sharedMemory == NULL){
    printf("ERROR: Couldn't get block.\n");
    return -1;
  }

  // ======= Initialise shared memory values =======
  sharedMemory->iterations = 0;
  sharedMemory->numOfReads = sharedMemory->numOfWrites = 0;
  sharedMemory->pageFaults = sharedMemory->hits = 0;
  sharedMemory->gccTraceBits = sharedMemory->bzipTraceBits = 0;

  // ======= Other variables =======
  unsigned int traceAddress;
  char traceType;
  int contiguousTrace = 0;

  // Each copy will make a copy of page &
  // table structs with same size
  Page *page;
  HashTable *table = createHashTable(CAPACITY);

  pid_t pid = fork();

  // P1 will create the queue (shared page table)
  if(pid != 0)
    sharedMemory->queue = createQueue(framesNumber);

  switch (chosenAlgorithm)
  {
    case 0:
      // LRU Algorithm Implementation
      if(pid != 0){
        // Parent process ==> P1

        // File to read
        FILE *gccFile;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        if ((gccFile = fopen("gcc.trace", "r")) == NULL)
        {
          printf("Couldn't open gcc.trace.\n");
          return 2;
        }

        while (sharedMemory->iterations < maxTraceBits && (read = getline(&line, &len, gccFile)) != -1)
        {
          sem_wait(sem_p1);
          
          for(contiguousTrace = 0; contiguousTrace < q; contiguousTrace++){

            if(sharedMemory->iterations >= maxTraceBits) break;

            sscanf(line, "%x %c", &traceAddress, &traceType);
            page = createPage(traceAddress, traceType, PROCESS1_ID);
            if(!referToPageInQueue(sharedMemory->queue, table, page)){
              sem_wait(mutex);
              sharedMemory->pageFaults++;
              sem_post(mutex);
            } else {
              sem_wait(mutex);
              sharedMemory->hits++;
              sem_post(mutex);
            }

            // Increment Read/Write counter
            // depending on the trace type
            if(page->traceType == 'R'){
              sem_wait(mutex);
              sharedMemory->numOfReads++;
              sem_post(mutex);
            } else {
              sem_wait(mutex);
              sharedMemory->numOfWrites++;
              sem_post(mutex);
            }

            // Increase the counter of the current process trace reads (gcc.trace)
            sharedMemory->gccTraceBits++;

            sem_wait(mutex);
            sharedMemory->iterations++;
            sem_post(mutex);

          }

          sem_post(sem_p2);
        }

        // ======= End of program - Print the statistics =======

        sem_wait(mutex);
        printf("\nqueue size is %d", sharedMemory->queue->occupiedPages);
        sem_post(mutex);

        sem_wait(mutex);
        printf("\nP1 Hash Table:");
        printHashTable(table);
        printf("Occupied pages in table 1: %d\n", table->totalPages);

        printQueue(sharedMemory->queue);
        sem_post(mutex);
        deleteHashTable(table);

        // Wait for P2 to finish
        wait(NULL);

        printf("Total iterations: %d\n", sharedMemory->iterations);
        printf("Total P1 (gcc) iterations: %d\n", sharedMemory->gccTraceBits);
        printf("Total P2 (bzip) iterations: %d\n", sharedMemory->bzipTraceBits);
        printf("Total page faults: %d\n", sharedMemory->pageFaults);
        printf("Total hits: %d\n", sharedMemory->hits);
        printf("Total reads: %d\n", sharedMemory->numOfReads);
        printf("Total writes: %d\n", sharedMemory->numOfWrites);
      }
      else
      {
        // Child process ==> P2

        // File to read
        FILE *bzipFile;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        if ((bzipFile = fopen("bzip.trace", "r")) == NULL)
        {
          printf("Couldn't open bzip.trace.\n");
          return 2;
        }

        while (sharedMemory->iterations < maxTraceBits && (read = getline(&line, &len, bzipFile)) != -1)
        {
          sem_wait(sem_p2);

          for(contiguousTrace = 0; contiguousTrace < q; contiguousTrace++){

            if(sharedMemory->iterations >= maxTraceBits) break;

            sscanf(line, "%x %c", &traceAddress, &traceType);
            page = createPage(traceAddress, traceType, PROCESS2_ID);
            insertToHashTable(table, page);
            // if (!referToPageInQueue(sharedMemory->queue, table, page))
            // {
            //   sem_wait(mutex);
            //   sharedMemory->pageFaults++;
            //   sem_post(mutex);
            // }
            // else
            // {
            //   sem_wait(mutex);
            //   sharedMemory->hits++;
            //   sem_post(mutex);
            // }

            // Increment Read/Write counter
            // depending on the trace type
            if (page->traceType == 'R') {
              sem_wait(mutex);
              sharedMemory->numOfReads++;
              sem_post(mutex);
            } else {
              sem_wait(mutex);
              sharedMemory->numOfWrites++;
              sem_post(mutex);
            }

            // Increase the counter of the current process trace reads (bzip.trace)
            sharedMemory->bzipTraceBits++;

            sem_wait(mutex);
            sharedMemory->iterations++;
            sem_post(mutex);
          }

          sem_post(sem_p1);
        }
        sem_wait(mutex);
        printf("\nP2 Hash Table:");
        printHashTable(table);
        printf("Occupied pages in table 2: %d\n", table->totalPages);
        sem_post(mutex);
        deleteHashTable(table);
      }

      // wait(NULL);

      break;
    case 1:
      // Second Chance Algorithm

      break;
    default:
      break;
  }

  // ========= Deletions =========
  sem_unlink(MUTEX);
  sem_unlink(SEM_P1);
  sem_unlink(SEM_P2);

  detachBlock(sharedMemory);

  // Delete the shared memory after it's no longer used
  if (destroyBlock(FILENAME, sizeof(Block), SHARED_MEMORY_ID))
  {
    printf("Destroyed block [%d]\n", SHARED_MEMORY_ID);
  }
  else
  {
    printf("Couldn't destroy block [%d]\n", SHARED_MEMORY_ID);
    return -1;
  }
  return 0;
}