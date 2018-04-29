/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

/* static unsigned char *freeptr; */

struct freeBlock {
  int index;
  int size;
  struct freeBlock *next;
  struct freeBlock *previous;
};

static struct freeBlock *first = NULL;

void deleteNode(struct freeBlock *del) {
  if (del == NULL) return;
  if (del->previous == NULL && del->next == NULL) {
    first = NULL;
  }
  else if (del->previous == NULL) {
    first = del->next;
    first->previous = NULL;
  }
  else if (del->next == NULL) {
    del->previous->next = NULL;
  }
  else {
    del->previous->next = del->next;
    del->next->previous = del->previous;
  }
  myfree((unsigned char *)del);
}

void addNode(int ind, int sizeBlock) {
  struct freeBlock *newNode = (struct freeBlock *)myalloc(sizeof(struct freeBlock));
  printf("%p\n", newNode);
  newNode->index = ind;
  printf("1\n");

  newNode->size = sizeBlock;
  printf("2\n");

  if (first == NULL) {
    first = newNode;
    first->next = NULL;
    first->previous = NULL;
    return;
  }
  printf("2\n");

  struct freeBlock *find = first;
  
  while (find->next != NULL && find->size < sizeBlock) {
    find = find->next;
  }
    printf("3\n");

  if (find->next == NULL) {
    if(find->size > sizeBlock) {
      newNode->next = find;
      newNode->previous = find->previous;
      find->previous = newNode;
      find->previous->next = newNode;
    }
    else {
      newNode->next = NULL;
      newNode->previous = find;
      find->next = newNode;
    }      
  }
  else {
    newNode->previous = find->previous;
    newNode->next = find;
    find->previous->next = newNode;
    find->previous = newNode;
  }
}

void changeNode(struct freeBlock *change, int newIndex, int newSize) {

  // NEED To check when things are near the beg/end
  change->index = newIndex;
  change->size = newSize;
  while (newSize > change->next->size) {
    change->next->previous = change->previous;
    change->previous->next = change->next;
    change->previous = change->next;
    change->next = change->previous->next;
    change->next->previous = change;
    change->previous->next = change;
    
  }
  while (newSize < change->previous->size) {
    change->previous->next = change->next;
    change->next = change->previous;
    change->previous = change->previous->previous;

    change->previous->next = change;

    change->next->previous = change;

    change->next->next->previous = change->next;
  }
}





/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {
    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *)malloc(MEMORY_SIZE);

    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
                MEMORY_SIZE);
        abort();
    }
    *((int *)mem) = MEMORY_SIZE - 8;
    *((int *)(mem + MEMORY_SIZE - 4)) = MEMORY_SIZE - 8;
    // struct freeBlock *del = first;
    //  struct freeBlock *nxt;
    /* while (del != NULL) {
      nxt = del->next;
      free(del);
      del = nxt;
    }
    first = NULL;
    addNode(0, MEMORY_SIZE - 8);*/
    
    /* freeptr = mem;*/
}

int findBestFit(int size) {
  int start = -1;
  int i = 0;
  int startSize = MEMORY_SIZE;
  while(i < MEMORY_SIZE) {
    if (*((int *)(mem + i)) > 0 && size < *((int *)(mem + i))) {
      if (*((int *)(mem + i)) < startSize) {
	start = i;
	startSize = *((int *)mem);
      }
    }
    i += abs(*((int *)(mem+ i))) + 8;
   }
  return start;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size) {
  //if (first == NULL) return 0;
  //struct freeBlock *startB = first;
  // while (startB != NULL && startB->size < size) startB = startB->next;
  // if (startB == NULL) return 0;
  
  // int start = startB->index;
 
  //int origSize = *((int *)(mem + start));
  int start = findBestFit(size);
  if (start < 0) return 0;
  if(*((int *)(mem + start))> 3*size) {
    *((int *)(mem + start + size + 8)) = *((int *)(mem + start)) - size - 8;
    *((int *)(mem + start + *((int *)(mem + start)) + 4)) = *((int *)(mem + start)) - size - 8;
    *((int *)(mem + start + size + 4)) = -size;
    *((int *)(mem + start)) = -size;
    // changeNode(startB, start + size + 8, origSize - size);
  }

  else {
    *((int *)(mem + start + *((int *)(mem + start)) + 4)) = -1*(*((int *)(mem + start)));
    *((int *)(mem + start)) = -1*(*((int *)(mem + start)));

    // deleteNode(startB);
   }

  return mem + start + 4;
  /*
    if (freeptr + size < mem + MEMORY_SIZE) {
        unsigned char *resultptr = freeptr;
        freeptr += size;
        return resultptr;
    }
    else {
        fprintf(stderr, "myalloc: cannot service request of size %d with"
                " %d bytes allocated\n", size, (freeptr - mem));
        return (unsigned char *) 0;
	}*/
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {
  // Check if change the things to -sign correctly
  int size = abs(*((int *)(oldptr - 4)));
  *((int *)(oldptr - 4)) = size;
  int nextBlockHead = -1;
  if (mem + MEMORY_SIZE < oldptr + size + 4) {
    nextBlockHead = *((int *)(oldptr + size + 4));
    if (nextBlockHead > 0) {
      *((int *)(oldptr - 4)) = size + nextBlockHead + 8;
      *((int *)(oldptr + size + 8 + nextBlockHead)) = size + nextBlockHead + 8;
    }
  }
  if (oldptr - 4 > mem) {
    int prevBlockFoot = *((int *)(oldptr - 8));
    if (prevBlockFoot > 0) {
      *((int *)(oldptr -12 - prevBlockFoot)) = *((int *)(oldptr - 4)) + prevBlockFoot + 8;
      if (nextBlockHead > 0) {
        *((int *)(oldptr + size + 8 + nextBlockHead)) = *((int *)(oldptr - 4)) + prevBlockFoot + 8;
      }
      else {
        *((int *)(oldptr + size)) = *((int *)(oldptr - 4)) + prevBlockFoot + 8;
      }
    }
   }
}
