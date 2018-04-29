/* ! \file
 * Declarations for a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004 - 2009.
 * All rights reserved.
 */


/* ! Specifies the size of the memory pool the allocator has to work with. */
extern int MEMORY_SIZE;

/* Finds the index of the free block that best fits a block of some size */
int findBestFit(int size);
  
/* Initializes allocator state, and memory pool state too. */
void init_myalloc();


/* Attempt to allocate a chunk of memory of "size" bytes. */
unsigned char * myalloc(int size);


/* Free a previously allocated pointer. */
void myfree(unsigned char *oldptr);

/* Coalesce a free block with the previous block if it's free */
void coalescePrevious(unsigned char *oldptr, unsigned short nextBlockHead,
                      unsigned short size);

/* Coalesce a free block with the next block if it's free */
unsigned short coalesceNext(unsigned char *oldptr, unsigned short size);
