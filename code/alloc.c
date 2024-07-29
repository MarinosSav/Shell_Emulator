#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct blockMetaData{
  size_t size;
  struct blockMetaData *next;
  int isFree;
}block;

size_t blockSpace = sizeof(block);
block *listHead = NULL, *listTail = NULL;
//TODO: If there is time, split and scanMerge
/*
void scanMerge(){

  block *currentBlock = listHead;

  if (!currentBlock) return;

  while(currentBlock->next){
    while (currentBlock->isFree && currentBlock->next->isFree){
      currentBlock->size += currentBlock->next->size;
      currentBlock->next =currentBlock->next->next;
    }
    currentBlock = currentBlock->next;
  }

}

void split(block *toSplit, size_t newSize){

  block *newBlock = toSplit + toSplit->size + newSize + blockSpace;

  newBlock->next = toSplit->next;
  newBlock->isFree = 1;
  newBlock->size = toSplit->size - newSize - sizeof(block);
  toSplit->next = newBlock;
  toSplit->size = newSize;

}
*/
block* findFree(size_t size){

  block *currentBlock = listHead;

  while(currentBlock){
    if (currentBlock->isFree){
      if (currentBlock->size >= size) break;
      /*if (currentBlock->size > size + sizeof(block)) {
        split(currentBlock, size);
        break;
      }
      */
    }
    currentBlock = currentBlock->next;
  }
  return currentBlock;

}

block initBlock(block *currentPoint, size_t size){
  currentPoint->size = size;
  currentPoint->next = NULL;
  currentPoint->isFree = 0;
  return *currentPoint;
}

void* addBlock(size_t size){

  void *newPoint = sbrk(size + blockSpace);

  if (!newPoint || !size) return (void*) 0;

  block *newBlockPoint = newPoint;
  *newBlockPoint = initBlock(newBlockPoint, size);
  if (!listHead) listHead = newBlockPoint;
  else if (listTail) listTail->next = newBlockPoint;
  listTail = newBlockPoint;

  return newBlockPoint;
}


void *mymalloc(size_t size)
{

  size = size + 8 - size % 8;
  block *resultPoint = findFree(size);

  if (resultPoint) resultPoint->isFree = 0;
  else resultPoint = addBlock(size);

  //(block*) ((((long) (resultPoint + 1)) + 0x7) & (~ 0x7))
  return (block*)(resultPoint + 1);

}

void *mycalloc(size_t nmemb, size_t size)
{

  if ((nmemb * size) == 0) return (void*) 0;
  void* currentBlock = mymalloc(nmemb * size);
  memset(currentBlock, 0, size * nmemb);

  return currentBlock;
}

void myfree(void *ptr)
{
  if(!ptr) return;

  block* currentPoint = (block*)ptr - 1;

  if(((void*) ptr + currentPoint->size) >= sbrk(0)){
    if(listHead == listTail) listHead = listTail = NULL;
    else{
      block* currentBlock = listHead;
      while(currentBlock){
        if(currentBlock->next == listTail){
          currentBlock->next = NULL;
          listTail = currentBlock;
        }
        currentBlock = currentBlock->next;
      }
    }
    sbrk(0 - currentPoint->size - blockSpace);
    return;
  }
  currentPoint->isFree = 1;
  //scanMerge();
  return;
}


void *myrealloc(void *ptr, size_t size){

  if(!size || !ptr) return mymalloc(size);

  block* currentBlock = (block*) ptr - 1;

  if (currentBlock->size >= size) {
    //split(currentBlock, size);
    return ptr;
  }

  void *tempBlock = mymalloc(size);

  if (tempBlock){
    memcpy(tempBlock, ptr, currentBlock->size);
    myfree(ptr);
  }
  return tempBlock;

}


/*
 * Enable the code below to enable system allocator support for your allocator.
 */
 #if 1
 void *malloc(size_t size) { return mymalloc(size); }
 void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
 void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
 void free(void *ptr) { myfree(ptr); }
 #endif
