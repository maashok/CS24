/*
 * Define a bounded buffer containing records that describe the
 * results in a producer thread.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "semaphore.h"

#include "sthread.h"
#include "bounded_buffer.h"

/*
 * The bounded buffer data.
 */
struct _bounded_buffer {
    /* The maximum number of elements in the buffer */
    int length;

    /* The index of the first element in the buffer */
    int first;

    /* The number of elements currently stored in the buffer */
    int count;

    /* The values in the buffer */
    BufferElem *buffer;
};

/*
 * For debugging, ensure that empty slots in the buffer are
 * set to a default value.
 */
static BufferElem empty = { -1, -1, -1 };

/* Stores whether a thread is already running in a buffer
 * function */
static Semaphore *oneAccess = NULL;

/* Stores the number of elements stored in the buffer */
static Semaphore *freeSpots = NULL;

/* Stores the number of free spaces in the buffer */
static Semaphore *filledSpots = NULL;

/*
 * Allocate a new bounded buffer.
 */
BoundedBuffer *new_bounded_buffer(int length) {
    if (oneAccess == NULL)
       oneAccess = new_semaphore(1);

    /* If another thread is already executing in any of the
     * buffer functions, wait so that shared data (buffer)
     * is not changed simultaneously, causing unexpected results
     * This also ensures that another thread doesn't start
     * using the buffer before it is completely initialized
     */
    semaphore_wait(oneAccess);
    
    BoundedBuffer *bufp;
    BufferElem *buffer;
    int i;

    /* Allocate the buffer */
    buffer = (BufferElem *) malloc(length * sizeof(BufferElem));
    bufp = (BoundedBuffer *) malloc(sizeof(BoundedBuffer));
    if (buffer == 0 || bufp == 0) {
        fprintf(stderr, "new_bounded_buffer: out of memory\n");
        exit(1);
    }

    /* Initialize */

    memset(bufp, 0, sizeof(BoundedBuffer));

    for (i = 0; i != length; i++)
        buffer[i] = empty;

    bufp->length = length;
    bufp->buffer = buffer;

    /* Allocate memory for the semaphores used for passive
     * waiting for the buffer to fill up or empty */
    if (freeSpots == NULL)
        freeSpots = new_semaphore(bufp->length);
    if (filledSpots == NULL)
        filledSpots = new_semaphore(0);

    /* Signal that this thread is finished completing this function
     * and that another thread may start execution if wanted */
    /* This thread must signal so that it does not continue holding
     * the control throughout its execution, and so that other threads
     * also get a chance to add and take integers from the buffer */
    semaphore_signal(oneAccess);
    return bufp;
}

/*
 * Add an integer to the buffer.  Yield control to another
 * thread if the buffer is full.
 */
void bounded_buffer_add(BoundedBuffer *bufp, const BufferElem *elem) {
    /* Wait if the buffer has no free spots. To avoid a busy
     * until the buffer has some space to add another element, 
     * just block the thread. Once some other thread removes
     * an element from the buffer, this thread will get a chance
     * to continue execution and add an element */
    semaphore_wait(freeSpots);

    /* Wait if another thread is already accessing. This way, the
     * buffer isn't affected by multiple threads at the same time, 
     * leading to unpredictable changes in the buffer */
    semaphore_wait(oneAccess);

    /* Now the buffer has space */
    bufp->buffer[(bufp->first + bufp->count) % bufp->length] = *elem;
    bufp->count++;

    /* Signal that this operation has caused one more spot to
     * be filled as well. This way, if some other thread is
     * waiting for an element to be added so that it can take it,
     * that element can know to continue execution */
    semaphore_signal(filledSpots);

    /* Yield control of execution the buffer functions. This
     * thread has had a chance to run a function, and should
     * allow other threads to as well, without fear of changing
     * their collective buffer at the same time */
    semaphore_signal(oneAccess);
}

/*
 * Get an integer from the buffer.  Yield control to another
 * thread if the buffer is empty.
 */
void bounded_buffer_take(BoundedBuffer *bufp, BufferElem *elem) {
    /* Wait if the buffer has no filled spots. To avoid a busy
     * until the buffer has some elements in it, 
     * just block the thread. Once some other thread adds
     * an element to the buffer, this thread will get a chance
     * to continue execution and take an element */
    semaphore_wait(filledSpots);

    /* Wait if another thread is already accessing. This way, the
     * buffer isn't affected by multiple threads at the same time, 
     * leading to unpredictable changes in the buffer */
    semaphore_wait(oneAccess);

    /* Copy the element from the buffer, and clear the record */
    *elem = bufp->buffer[bufp->first];
    bufp->buffer[bufp->first] = empty;
    bufp->count--;
    bufp->first = (bufp->first + 1) % bufp->length;

    /* Signal that this operation has caused one more spot to
     * be freed as well. This way, if some other thread is
     * waiting for an element to be taken away so that it can now
     * add something, that element can know to continue execution */
    semaphore_signal(freeSpots);

    /* Yield control of execution the buffer functions. This
     * thread has had a chance to run a function, and should
     * allow other threads to as well, without fear of changing
     * their collective buffer at the same time */
    semaphore_signal(oneAccess);
}
