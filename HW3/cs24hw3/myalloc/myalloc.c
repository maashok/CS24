/* ! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004 - 2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "myalloc.h"

/* Store the size of the header and footer and the header + footer */
#define SIZEH 3
#define SIZEF 2
#define SIZETOT 5

/* !
 * The format of each memory block is a 3 byte header - the first byte is a 
 * character which is 1 if the block is being used and 0 if it is free.
 * The next 2 bytes are an unsigned short holding the size of the payload.
 * Then comes the payload. After the payload, there is a 2 byte usigned short
 * holding the size of the payload as well (To allow for constant time 
 * deallocation).
 * 
 * The header and footer were stored as unsigned shorts with a 1 byte 
 * free/alloc'ed flag instead of a struct to maximize data utilization, as
 * structs will pad the memory usage to word alignment. Instead this way,
 * we minimize the headers and footers to a maximum of 3, instead of 4 bytes.
 */

/* !
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated 
 * within init_myalloc(), and then myalloc() and free() work against this
 * pool of memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

/* !
 * This function initializes both the allocator state, and the memory pool.
 * It must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 *
 * Initializing the allocaotr takes constant, or O(1) time.
 */
void init_myalloc() {
    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *)malloc(MEMORY_SIZE);

    /*
     * Checks if it could be allocated
     */
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
                MEMORY_SIZE);
        abort();
    }
    
    /* 
     * Assigns the header and the footer of the entire memory pool.
     * This is free at first (1st byte = 0), and the other 2 bytes of
     * the header and the footer are set to the memory left for
     * data storage
     */
    unsigned short memForData = (unsigned short)(MEMORY_SIZE - SIZETOT);
    *((unsigned short *)(mem + 1)) = memForData;
    *mem = 0;
    *((unsigned short *)(mem + MEMORY_SIZE - SIZEF)) = memForData;
}


/* !
 *
 * This function helps with best - fit placement strategy to limit memory
 * fragmentation. Given the size in bytes of the memory we want to
 * allocate as a parameter, this finds the smallest free block that
 * can fit this new block of memory. It returns a pointer to the
 * header of the best free block (3 bytes before payload) that it finds.
 * If there is no free block that can fit the block we want to allocate,
 * returns -1.

 * This operation of finding the best fit takes O(n) time, where n is the
 * number of free blocks in our memory pool.
 */

int findBestFit(int size) {
  /* 
   * Starts from the first block - which always starts at
   * the first index of the memory pool
   */
  int start = -1;
  int i = 0;
  int startSize = MEMORY_SIZE;
  
  /* Goes through all the blocks in the memory pool */
  while(i < MEMORY_SIZE) {
    /* Stores the size of the current block */
    unsigned short thisSize = *((unsigned short *)(mem + i + 1));
    /* 
     * If the current block is free, bigger than the block
     * we want to allocate space for, and smaller than any other
     * block we have seen so far that works, then make this 
     * the best one.
     */
    if (*(mem + i) == 0) {
      if ((size + SIZETOT) < thisSize) {
        if (thisSize < startSize) {
          start = i;
          startSize = thisSize;
        }
      }
    }
    /* 
     * To get to the next block, add on the size of this block
     * and the size of the header and footer.
     */
    i += thisSize + SIZETOT;
  }
  /* Returns the index of the best block */
  return start;
}


/* !
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails, and the pointer to the first byte of the payload if 
 * it works.
 *
 * Uses best - fit placement strategy to limit the memory fragmentation. 
 * This will limit fragmentation in most cases since we pick the smallest
 * free block of memory that we have that can satisfy the size requirements 
 * for this allocation.  However, this strategy will take more time than
 * needed if we have many allocations all at one point and then this block 
 * of memory is freed at the same time. Then, since we will end up allocating 
 * memory sequentially, it would have taken less run time to use first - fit
 * or next - fit strategy. This strategy will mainly be good when the 
 * allocations have varying timescales.
 *
 *
 * The operation of finding the best - fit block takes linear time in the 
 * number of free blocks, while updating the headers and footers of 
 * the block we want to use and any surrounding blocks we affect takes 
 * constant time.
 *
 * Thus, overall, allocation takes O(n) time, where n is the number of
 * free blocks.
 */
unsigned char *myalloc(int size) {
  /* 
   *Finds the best fit block to minimize fragmentation and checks if the
   * index found is -1 - which means there was no space.
   */
  int start = findBestFit(size);
  if (start < 0) return 0;
  
  /* The original size of the block is what is in it's header */
  unsigned short sizeOrig = *((unsigned short *)(mem + start + 1));

  /* 
   * If the size of the block is more than 1.5 times the size of
   * the block we want to allocate, then split it up.
   */
  if(sizeOrig * 2 > 3 * (size + SIZETOT)) {
    /* 
     * The size left in the free block is the original size minus
     * the size of the allocated block and any other header/footer.
     */
    unsigned short sizeLeft = (unsigned short)(sizeOrig - size - SIZETOT);

    /* 
     * Update the free block's header and then footer to signify
     * that it is free, and that its size is the size left.
     */
    *((unsigned short *)(mem + start + size + SIZETOT + 1)) = sizeLeft;
    *(mem + start + size + SIZETOT) = 0;
    *((unsigned short *)(mem + start + sizeOrig + SIZEH)) = sizeLeft;

    /* 
     * Update the allocated part's header and footer to show that its size is
     * reduced to only the size it needs.
     */
    *((unsigned short *)(mem + start + size + SIZEH)) = (unsigned short)size;
    *((unsigned short *)(mem + start + 1)) = (unsigned short)size;
  }

  
  /* 
   * The header of the allocated block will be changed to 1 to signify
   * that it is no longer free.
   */
  *(mem + start) = 1;
  /* Returns a pointer to the first byte of the payload */
  return mem + start + SIZEH;
}


/* !
 * Free a previously allocated pointer.  oldptr should be an address
 * returned by myalloc().
 *
 * Freeing the memory takes constant time, as it is just changing the flag
 * in the header of the memory block to 0 to signify it is not being used.
 * Coalescing the blocks takes constant time as well, as we have a header
 * and footer associated with each object. Using this, to deallocate, we
 * just move as many bytes back to the previous and forward to the next
 * block if they are free and coalesce them, instead of having to make
 * a pass through our entire memory pool to coalesce free blocks.
 *
 * Thus, the overall time for deallocation and coalescing blocks is O(1)
 * or constant time.
 */
void myfree(unsigned char *oldptr) {
  /* 
   * Checks if the block of memory that the pointer passed in points to
   * has already been freed. If so, prints error message and aborts */
  if (*(oldptr - SIZEH) == 0) {
    fprintf(stderr, "myfree: Freeing memory that has already been freed\n");
    abort();
  }
  
  /* Finds the size of the block that oldptr points to */
  unsigned short size = *((unsigned short *)(oldptr - SIZEH + 1));
  /* This block's header is changed to have a free flag */
  *(oldptr - SIZEH) = 0;

  /* 
   * Will try to coalesce with the next block, given the
   * current block, and store the size specified in the
   * next block header.
   */
  unsigned short nextBlockHead = coalesceNext(oldptr, size);

  coalescePrevious(oldptr, nextBlockHead, size);
}


/* !
 * This function tries to coalesce the memory block pointed to by
 * the parameter oldptr with the previous memory block, and doesn't 
 * return anything.
 *
 * This operation takes constant time or O(1) as it just uses
 * the sizes specified in the headers and footers to access the 
 * correct memory address without looping through the memory pool.
 */
void coalescePrevious(unsigned char *oldptr, unsigned short nextBlockHead,
                      unsigned short size) {
  /* 
   * Tries to coalesce with the previous block if this is not
   * the first block in memory */
  if (oldptr - SIZEH > mem) {
    /* The size of the previous block can be found in its footer */
     unsigned short prevBlockHead = *((unsigned short *)(oldptr - SIZETOT));
     
     /* 
      * Using the size of the previous block, we can move back to its header,
      * where we can check that it's free or available */
     if (*(oldptr - SIZETOT - SIZEH - prevBlockHead) == 0) {
       /* The new size of the coalesced block is determined */
       unsigned short newSize;
       
       /* If we already coalesced with the next block */
        if (nextBlockHead < 0xFFFF) {
          /* 
           * The new size is that of the original block, the previous, next,
           * and unneeded header/footer bytes */
          newSize = size + nextBlockHead + prevBlockHead + 2 * SIZETOT;

          /* 
           * If we already coalesced with the next block, our header
           * is at the start of the previous block and the footer is at 
           * the end of the next block for the new coalesced block.
           */
          *((unsigned short *)(oldptr - SIZETOT - SIZEF - prevBlockHead))
            = newSize;
          *((unsigned short *)(oldptr + size + SIZETOT + nextBlockHead))
            = newSize;
        }
        
        /* If we haven't coalesced with the next block */
        else {
          /* 
           * The new size is that of just the original plus the
           * previous block and header/footer space */
          newSize = size + prevBlockHead + SIZETOT;

          /* 
           *The header is at the start of the previous block and the
           * footer is at the end of the current block we are freeing.
           */
          *((unsigned short *)(oldptr - SIZETOT - SIZEF - prevBlockHead))
            = newSize;
          *((unsigned short *)(oldptr + size)) = newSize;
        }
     }
   }
}


/* !
 * This function tries to coalesce the memory block pointed to by
 * the parameter oldptr with the next memory block.
 * If this works, this function will return the size of the next
 * block before coalescing. If it does not work, it will return
 * 0xFFFF, a size that is likely not going to happen in real
 * allocation scenarios.
 *
 * This operation takes constant time or O(1) as it just uses
 * the sizes specified in the headers to access the correct memory
 * address without looping through the memory pool.
 */

unsigned short coalesceNext(unsigned char *oldptr, unsigned short size) {
  /* 
   * We will first initialize the size of the block after it
   * to 0xFFFF, a value we will not likely use to distinguish
   * whether we have coalesced with the next block or not
   */
  unsigned short nextBlockHead = 0xFFFF;
  
  /* 
   * Tries to coalesce with the next block if this is not the
   * last block in the pool */
  if (mem + MEMORY_SIZE < oldptr + size + SIZEF) {
    /* Checks if the next block is free */
    if (*(oldptr + size + SIZEF) == 0) {
      
      /* The next block's size is given by its header */
      nextBlockHead = *((unsigned short *)(oldptr + size + SIZEH));
      
      /* 
       * The header of the original block is increased to include the extra
       * size of the next block plus header/footer space */
      unsigned short extraSize = size + nextBlockHead + SIZETOT;
      *((unsigned short *)(oldptr - SIZEF)) = extraSize;
      /* The footer of the next block is also increased to the same value */
      *((unsigned short *)(oldptr + size + SIZETOT + nextBlockHead))
        = extraSize;
    }
  }
  return nextBlockHead;
}
  
