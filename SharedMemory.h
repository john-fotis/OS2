#ifndef __SHARED__MEMORY__
#define __SHARED__MEMORY__

#include "SimulationLibrary.h"

#define IPC_ERROR (-1)

typedef struct {
  // Statistics variables
  int iterations, numOfReads, numOfWrites;
  int pageFaults, hits;
  int gccTraceBits, bzipTraceBits;
  // Queue
  Queue *queue;
} Block;

Block *attachBlock(char *fileName, int size, unsigned int proj_id);
int detachBlock(Block *block);
int destroyBlock(char *blockName, int size, unsigned int proj_id);

// Filename of shared memory blocks
#define FILENAME (char *)"main.c"
// IDs of shared memory blocks
#define SHARED_MEMORY_ID 1
// Semaphore names
#define MUTEX "/mutex"
#define SEM_P1 "/p1"
#define SEM_P2 "/p2"

#endif