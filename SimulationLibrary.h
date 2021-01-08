#ifndef __SIMULATIONLIBRARY__
#define __SIMULATIONLIBRARY__

#define CAPACITY 100  // Size of the Hash Tables
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096

// Basic struct used in HashTable, List and Queue
typedef struct Page {
  unsigned int trace;
  unsigned int pageNumber;
  unsigned int offset;
  char traceType;
  unsigned int proccessId;
  // isInCache is 0 when the page is not in cache
  // and 1 when present in cache
  int isInCache;
  int secondChance;
} Page;

// Linkedlist
typedef struct LinkedList {
  Page *page;
  struct LinkedList *next;
} LinkedList;

// Hash Table
typedef struct HashTable {
  // An array of pointers to HashNodes
  Page **pageArray;
  // An array of buckets to solve collision effect
  LinkedList **overflowBucket;
  int size;
  int count;
  int totalPages;
} HashTable;

// FIFO Queue
typedef struct Queue {
  unsigned occupiedPages;
  unsigned maxPages;
  int front, rear;
  Page *pageArray;
} Queue;

// Page Functions
Page *createPage(unsigned int trace, char type, unsigned int pid);
void deletePage(Page *nodeToDelete);
void printPage(HashTable *table, Page *page);

// List Functions
LinkedList *createList();
LinkedList *insertToList(LinkedList *list, Page *newNode);
void deleteList(LinkedList *list);
LinkedList **createOverflowBucket(HashTable *table);
void deleteOverflowBucket(HashTable *table);

// HashTable Functions
unsigned long hashFunction(unsigned int trace);
HashTable *createHashTable(int size);
void deleteHashTable(HashTable *table);
void handleCollision(HashTable *table, unsigned long index, Page *collidedNode);
void insertToHashTable(HashTable *table, Page *page);
Page *searchHashTable(HashTable *table, unsigned int trace, int pid);
void printHashTable(HashTable *table);

// Queue Functions
Queue *createQueue(int numberOfPages);
int isQueueFull(Queue *queue);
int isQueueEmpty(Queue *queue);
void insertToQueue(Queue *queue, Page *page);
int searchPageInQueue(Queue * queue, Page *page);
int lruReferToPageInQueue(Queue *queue, HashTable *table, Page *page);
int secondChanceReferToPageInQueue(Queue *queue, HashTable *table, Page *page);
void printQueue(Queue *queue);

#endif