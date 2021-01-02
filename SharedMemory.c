#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "SharedMemory.h"

static int getSharedBlock(char *fileName, int size, unsigned int proj_id) {
  key_t key;

  // Request a key and link it to a filename
  key = ftok(fileName, proj_id);
  if (key == IPC_ERROR) return IPC_ERROR;
  
  // Create block or get it if it exists
  return shmget(key, size, IPC_CREAT | 0644);
}

Block *attachBlock(char *fileName, int size, unsigned int proj_id) {
  
  Block *result;

  // Request the shared block id
  int sharedBlockId = getSharedBlock(fileName, size, proj_id);
  if (sharedBlockId == IPC_ERROR) return NULL;

  // Map the shared block to the current proccess
  // and return a pointer to it
  result = (Block *)shmat(sharedBlockId, NULL, 0);
  if (result == (Block *)IPC_ERROR) return NULL;

  return result;
}

int detachBlock(Block *block) {
  return (shmdt(block) != IPC_ERROR);
}

int destroyBlock(char *fileName, int size, unsigned int proj_id) {
  int sharedBlockId = getSharedBlock(fileName, size, proj_id);

  if (sharedBlockId == IPC_ERROR) return IPC_ERROR;

  return (shmctl(sharedBlockId, IPC_RMID, NULL) != IPC_ERROR);
}