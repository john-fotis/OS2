#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "SimulationLibrary.h"
#include "ThreadRoutines.h"

int main (int argc, char * argv[]) {

  // Check input validity
  if (!(argc == 4 || argc == 5)) {
    printf("Correct usage:\n \
    ./main.ex [0: LRU | 1: SECOND CHANCE] \
    [RAM frames number] \
    [Sequential trace entries for each process (q)] \
    (Max number of trace characters to read) \n");
    return 1;
  }

  // Get execution arguments
  int chosenAlgorithm = atoi(argv[1]);
  if (chosenAlgorithm != 0 && chosenAlgorithm != 1){
    printf("Invalid Algorithm given.\nSelect 0 for LRU or 1 for Second Chance.\n");
    return -1;
  }
  int framesNumber = atoi(argv[2]);
  if (framesNumber < 2) {
    printf("Frames number must be greater than 2\n");
    return -1;
  }
  int q = atoi(argv[3]);
  int maxTraceEntries = RAND_MAX;
  if (argc == 5) maxTraceEntries = atoi(argv[4]);

  SharedBlock *sharedMemory = calloc(1, sizeof(SharedBlock));

  // Initialise shared memory values
  sharedMemory->chosenAlgorithm = chosenAlgorithm;
  sharedMemory->q = q;
  sharedMemory->maxTraceEntries = maxTraceEntries;
  sharedMemory->iterations = 0;
  sharedMemory->numOfReads = sharedMemory->numOfWrites = 0;
  sharedMemory->pageFaults = sharedMemory->hits = sharedMemory->updates = 0;
  sharedMemory->gccTraceBits = sharedMemory->bzipTraceBits = 0;
  // Create the queue (shared page table)
  sharedMemory->queue = createQueue(framesNumber);

  pthread_t t1, t2;

  // Create threads
  if(pthread_create(&t1, NULL, ThreadRoutine1, sharedMemory) != 0) return -1;
  if(pthread_create(&t2, NULL, ThreadRoutine2, sharedMemory) != 0) return -1;
  // Wait for them to finish execution
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  // ======= End of program - Print the statistics =======
  printf("\n>>>>>Memory status<<<<<\n");
  printQueue(sharedMemory->queue);
  printf("\nQueue size is %d\n", sharedMemory->queue->occupiedPages);

  printf("\nTotal iterations: %d\n", sharedMemory->iterations);
  printf("Total P1 (gcc) iterations: %d\n", sharedMemory->gccTraceBits);
  printf("Total P2 (bzip) iterations: %d\n", sharedMemory->bzipTraceBits);
  printf("Total page faults: %d\n", sharedMemory->pageFaults);
  printf("Total hits: %d\n", sharedMemory->hits);
  printf("Total updates: %d\n", sharedMemory->updates);
  printf("Total reads: %d\n", sharedMemory->numOfReads);
  printf("Total writes: %d\n", sharedMemory->numOfWrites);
  
  free(sharedMemory);

  return 0;
}