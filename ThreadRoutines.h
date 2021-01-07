#ifndef __THREADROUTINES__
#define __THREADROUTINES__

#include "SimulationLibrary.h"

#define PID_1 1
#define PID_2 2

typedef struct {
  // Program variables
  int chosenAlgorithm, q, maxTraceEntries;
  // Statistics variables
  int iterations, numOfReads, numOfWrites;
  int pageFaults, hits, updates;
  int gccTraceBits, bzipTraceBits;
  // Queue
  Queue *queue;
} SharedBlock;

void *ThreadRoutine1(void *arg);
void *ThreadRoutine2(void *arg);

#endif