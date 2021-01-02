#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SimulationLibrary.h"

// ============= List =============

LinkedList *createList()
{
  LinkedList *list = (LinkedList *)calloc(1, sizeof(LinkedList));
  return list;
}

LinkedList *insertToList(LinkedList *list, Page *newNode)
{
  if (!list)
  {
    LinkedList *head = createList();
    head->page = newNode;
    head->next = NULL;
    list = head;
    return list;
  }

  else if (list->next == NULL)
  {
    LinkedList *node = createList();
    node->page = newNode;
    node->next = NULL;
    list->next = node;
    return list;
  }

  LinkedList *temp = list;
  while (temp->next)
  {
    temp = temp->next;
  }

  LinkedList *node = createList();
  node->page = newNode;
  node->next = NULL;
  temp->next = node;

  return list;
}

Page *popFromList(LinkedList *list)
{
  // Removes the head from the linked list
  // and returns the item of the popped element
  if (!list)
    return NULL;
  if (!list->next)
    return NULL;
  LinkedList *node = list->next;
  LinkedList *temp = list;
  temp->next = NULL;
  list = node;
  Page *nodeToPop = NULL;
  memcpy(temp->page, nodeToPop, sizeof(Page));
  free(temp->page);
  free(temp);
  return nodeToPop;
}

void deleteList(LinkedList *list)
{
  LinkedList *temp = list;
  if (!list)
    return;
  while (list)
  {
    temp = list;
    list = list->next;
    free(temp->page);
    free(temp);
  }
}

LinkedList **createOverflowBucket(HashTable *table)
{
  LinkedList **bucket = (LinkedList **)calloc(table->size, sizeof(LinkedList *));
  for (int i = 0; i < table->size; i++)
    bucket[i] = NULL;
  return bucket;
}

Page *createPage(unsigned int key, char type, unsigned int pid)
{
  Page *node = (Page *)malloc(sizeof(Page));

  node->key = key;
  node->pageNumber = node->key / PAGE_SIZE;
  node->offset = node->pageNumber % PAGE_SIZE;
  node->traceType = type;
  node->proccessId = pid;
  node->isInCache = 0;

  return node;
}

void deletePage(Page *nodeToDelete)
{
  free(nodeToDelete);
}

void printPage(HashTable *table, Page *page)
{
  if(page == NULL) return;

  if ((page = searchHashTable(table, page->key, page->proccessId)) == NULL)
  {
    printf("%d does not exist\n", page->key);
    return;
  }
  else
  {
    printf("Key:%8x | Page Number:%8d | Offset:%6d | Type:%2c", page->key, page->pageNumber, page->offset, page->traceType);
  }
}

void deleteOverflowBucket(HashTable *table)
{
  LinkedList **bucket = table->overflowBucket;
  for (int i = 0; i < table->size; i++)
    deleteList(bucket[i]);
  free(bucket);
}

// ============= HashTable =============

unsigned long hashFunction(unsigned int key)
{
  unsigned long i = 0;
  i = ((key >> 16) ^ key) * 0x45d9f3b;
  i = ((i >> 16) ^ i) * 0x45d9f3b;
  i = ((i >> 16) ^ i);
  return i % CAPACITY;
}

HashTable *createHashTable(int size)
{
  // Creates a new HashTable
  HashTable *table = (HashTable *)malloc(sizeof(HashTable));
  table->size = size;
  table->count = 0;
  table->totalPages = 0;
  table->pageArray = (Page **)calloc(table->size, sizeof(Page *));
  for (int i = 0; i < table->size; i++)
    table->pageArray[i] = NULL;
  table->overflowBucket = createOverflowBucket(table);

  return table;
}

void deleteHashTable(HashTable *table)
{
  // Frees the table
  for (int i = 0; i < table->size; i++)
  {
    Page *item = table->pageArray[i];
    if (item != NULL)
      deletePage(item);
  }

  deleteOverflowBucket(table);
  free(table->pageArray);
  free(table);
}

void handleCollision(HashTable *table, unsigned long index, Page *collidedNode)
{
  LinkedList *head = table->overflowBucket[index];

  if (head == NULL)
  {
    // First item in list
    head = createList();
    head->page = collidedNode;
    table->overflowBucket[index] = head;
    table->totalPages++;
    return;
  }
  else
  {
    // Insert to the list
    table->overflowBucket[index] = insertToList(head, collidedNode);
    table->totalPages++;
    return;
  }
}

void insertToHashTable(HashTable *table, Page *page)
{
  // The node is already in the hash table
  if(searchHashTable(table, page->key, page->proccessId) != NULL) return;

  // Create the item
  Page *nodeToInsert = createPage(page->key, page->traceType, page->proccessId);

  // Compute the index
  int index = hashFunction(nodeToInsert->key);

  Page *current_item = table->pageArray[index];

  if (current_item == NULL)
  {
    // Key does not exist.
    if (table->count == table->size)
    {
      // Hash Table Full
      printf("Insert Error: Hash Table is full\n");
      // Remove the create item
      deletePage(nodeToInsert);
      return;
    }

    // Insert directly
    table->pageArray[index] = nodeToInsert;
    table->count++;
    table->totalPages++;
  }

  else
  {
    // Case 1: We only need to update node
    if (current_item->key == nodeToInsert->key)
    {
      if(current_item->traceType == 'R' && nodeToInsert->traceType == 'W')
      {
        printf("Updated item %d\n", current_item->key);
        current_item->traceType = 'W';
      }
      return;
    }
    else
    {
      // Case 2: Collision
      handleCollision(table, index, nodeToInsert);
      return;
    }
  }
}

Page *searchHashTable(HashTable *table, unsigned int key, int pid)
{
  // Searches the key in the hashtable
  // and returns NULL if it doesn't exist
  int index = hashFunction(key);
  Page *requestedNode = table->pageArray[index];
  LinkedList *head = table->overflowBucket[index];

  // Ensure that we move to items which are not NULL
  while (requestedNode != NULL)
  {
    if (requestedNode->key == key &&
        requestedNode->proccessId == pid)
      return requestedNode;
    if (head == NULL)
      return NULL;
    requestedNode = head->page;
    head = head->next;
  }
  return NULL;
}

void removePageFromTable(HashTable *table, unsigned int key)
{
  // Deletes an item from the table
  int index = hashFunction(key);
  Page *nodeToRemove = table->pageArray[index];
  LinkedList *head = table->overflowBucket[index];

  if (nodeToRemove == NULL)
  {
    // Does not exist. Return
    return;
  }
  else
  {
    if (head == NULL && nodeToRemove->key == key)
    {
      // No chain. Remove the item
      // and set table index to NULL
      table->pageArray[index] = NULL;
      deletePage(nodeToRemove);
      table->count--;
      table->totalPages--;
      return;
    }
    else if (head != NULL)
    {
      // External Chain exists
      if (nodeToRemove->key == key)
      {
        // Remove this item and set the head of the list
        // as the new item

        deletePage(nodeToRemove);
        LinkedList *node = head;
        head = head->next;
        node->next = NULL;
        table->pageArray[index] = createPage(node->page->key, node->page->traceType, node->page->proccessId);
        deleteList(node);
        table->totalPages--;
        table->overflowBucket[index] = head;
        return;
      }

      LinkedList *curr = head;
      LinkedList *prev = NULL;

      while (curr)
      {
        if (curr->page->key == key)
        {
          if (prev == NULL)
          {
            // First element of the chain. Remove the chain
            deleteList(head);
            table->overflowBucket[index] = NULL;
            return;
          }
          else
          {
            // This is somewhere in the chain
            prev->next = curr->next;
            curr->next = NULL;
            deleteList(curr);
            table->overflowBucket[index] = head;
            return;
          }
        }
        curr = curr->next;
        prev = curr;
      }
    }
  }
}

void printHashTable(HashTable *table)
{
  printf("\n==============\n");
  for (int i = 0; i < table->size; i++)
  {
    if (table->pageArray[i])
    {
      printf("||Index %4d|| ",i);
      printPage(table, table->pageArray[i]);
      if (table->overflowBucket[i])
      {
        LinkedList *head = table->overflowBucket[i];
        while (head)
        {
          printf("  ==>  ");
          printPage(table, head->page);
          head = head->next;
        }
      }
      printf("\n");
    }
  }
  printf("==============\n\n");
}

// ============= Queue =============

Queue *createQueue(int numberOfPages) {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  
  queue->occupiedPages = 0;
  queue->maxPages = numberOfPages;

  queue->front = queue->rear = 0;
  queue->pageArray = malloc(numberOfPages * sizeof(Page));

  // Initialise queue
  int i = 0;
  for(i = 0; i < queue->maxPages; i++){
    queue->pageArray[i].key = 0;
    queue->pageArray[i].pageNumber = 0;
    queue->pageArray[i].offset = 0;
    queue->pageArray[i].traceType = 'R';
    queue->pageArray[i].proccessId = 0;
    queue->pageArray[i].isInCache = 1;
  }

  return queue;
}

int isQueueEmpty(Queue *queue) {
  return queue->occupiedPages == 0;
}

int isQueueFull(Queue *queue) {
  return queue->occupiedPages == queue->maxPages;
}

// A function to add a page queue (cache)
// based on LRU algorithm (always to the front)
void insertToQueue(Queue *queue, Page *page) {

  if(!isQueueFull(queue))
    queue->rear++;

  // Shift everything one cell to the end
  int i = queue->rear;
  while(i > 0){
    queue->pageArray[i] = queue->pageArray[i - 1];
    i--;
  }
  
  // Insert the new page to the front
  queue->pageArray[queue->front].key = page->key;
  queue->pageArray[queue->front].pageNumber = page->pageNumber;
  queue->pageArray[queue->front].offset = page->offset;
  queue->pageArray[queue->front].traceType = page->traceType;
  queue->pageArray[queue->front].proccessId = page->proccessId;
  queue->pageArray[queue->front].isInCache = 1;
  page->isInCache = 1;
  
  if(!isQueueFull(queue))
    queue->occupiedPages++;

}

// Deletes rear node from queue
void popFromQueue(Queue *queue) {
  if (isQueueEmpty(queue)) return;

  // If there's more than 1 node:
  if (queue->front != queue->rear)
    queue->rear--;

  // Decrement number of occupied nodes in queue
  queue->occupiedPages--;
}

// Returns 1 if the page is in queue (cache)
// or -1 in case of failure
int searchPageInQueue(Queue *queue, Page *page) {

  if(page == NULL) return -1;

  int index = 0;
  while(index <= queue->rear){
    if (page->key == queue->pageArray[index].key &&
        page->pageNumber == queue->pageArray[index].pageNumber &&
        page->offset == queue->pageArray[index].offset &&
        page->proccessId == queue->pageArray[index].proccessId)
      return index;
    index++;
  }

  return -1;
}

// Returns 1 if page was in cache
// and 0 if we had a page fault
int referToPageInQueue(Queue *queue, HashTable *table, Page *page) {
  // Check if the page is the hashtable
  Page *temp = searchHashTable(table, page->key, page->proccessId);
  unsigned int key = 0;
  int pid = 0;
  int index = 0;
  Page *pageToRemove;

  if (!isQueueEmpty(queue)){
    key = queue->pageArray[index].key;
    pid = queue->pageArray[index].proccessId;
    pageToRemove = searchHashTable(table, key, pid);
    index = searchPageInQueue(queue, pageToRemove);
  }

  // If the page isn't in hashtable, this is te first time loading it
  if(temp == NULL){
    // Mark the last page in queue as "removed"
    pageToRemove->isInCache = 0;
    // Insert the new page both in hash and queue
    insertToHashTable(table, page);
    insertToQueue(queue, page);
    // This is a page fault
    return 0;
  }

  // The page is in hash table but not in queue
  if(page->isInCache){
    // Mark the last page in queue as "removed"
    pageToRemove->isInCache = 0;
    // Load the new page only in the queue
    insertToQueue(queue, page);
    // This is also a page fault
    return 0;
  } else {
    // We found the page in queue, just bring it to the front
    index = searchPageInQueue(queue, page);
    while(index > 0){
      // Shift all the pages up to the referred page to the end
      queue->pageArray[index] = queue->pageArray[index - 1];
      index--;
    }
    // Bring the requested page to the front
    queue->pageArray[queue->front].key = page->key;
    queue->pageArray[queue->front].pageNumber = page->pageNumber;
    queue->pageArray[queue->front].offset = page->offset;
    queue->pageArray[queue->front].traceType = page->traceType;
    queue->pageArray[queue->front].proccessId = page->proccessId;
    // This is a hit
    return 1;
  }
  return 0;
}

void printQueue(Queue *queue)
{
  if(isQueueEmpty(queue)) return;

  int i = 0;
  printf("Printing Queue:\n=============\n");
  while(i < queue->occupiedPages)
  {
    printf("Key: %8x | Page Number: %6d | Type: %2c\n", queue->pageArray[i].key, queue->pageArray[i].pageNumber, queue->pageArray[i].traceType);
    i++;
  }
  printf("=============\n");
}