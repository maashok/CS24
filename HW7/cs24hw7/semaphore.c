/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "semaphore.h"

/* The semaphore uses a queue to store the blocked threads
 * since this ensures fairness. Queues are FIFO structures,
 * so that the thread that was blocked first will be
 * the first thread to be unblocked. Thus, every thread
 * will get the lock at some point, because no thread will
 * be skipped over in the queue, and they will also get it
 * in the order that they entered the queue in. As a result,
 * in most scenarios, all threads in the queue will also 
 * have to wait about the same amount of time to get the lock
 */

/* A queue element includes the thread it holds as well
 * as a pointer to the next queue element */
struct _queueElem {
  Thread *theThread;
  struct _queueElem *next;
};

/* A semaphore holds a value signifying how many more uses
 * of it are allowed before blocking, as well as the queue
 * of threads blocked due to waiting
 */
struct _semaphore {
  int value;

  struct _queueElem *first;
  struct _queueElem *last;
};

/* The queue is empty if the first element does not exist */
int queue_empty(Semaphore *curr) {
  return curr->first == NULL;
}

/* Adds a thread to the end of the blocked queue of 
 * the current semaphore
 */
void queue_add(Semaphore *curr, Thread *toAdd) {
  /* If the queue is empty, allocate memory for it
   * and make the first (and last) element the thread to add
   */
  if (curr->first == NULL) {
    curr->first = malloc(sizeof(struct _queueElem));
    curr->first->theThread = toAdd;
    curr->first->next = NULL;
    curr->last = curr->first;
  }
  /* Otherwise, just allocate memory for a new element
   * with the thread to add and add it to the end of the queue,
   * making it the last element */
  else {
    struct _queueElem *adding =
      malloc(sizeof(struct _queueElem));
    adding->theThread = toAdd;
    curr->last->next = adding;
    curr->last = curr->last->next;
  }
}

/* Takes an element from the front of the queue
 * and releases the memory for it */
Thread *queue_take(Semaphore *curr) {
  struct _queueElem *temp = curr->first;
  /* If the queue is empty, there is nothing to remove */
  if (temp == NULL)
    return NULL;
  /* Otherwise, remove the thread in the first element
   * and free the memory for the first element */
  else {
    Thread *toRet = temp->theThread;
    curr->first = temp->next;
    return toRet;
  }
}
    


/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {
    Semaphore *semp;
    /* Allocate memroy for the semaphore and set the initial value */
    semp = malloc(sizeof(Semaphore));
    semp->value = init;

    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {
  /* Ensure that the waiting operation is atomic and there are no
   * interrupts in between that cause undefined behavior. For example,
   * in a scenario where the semaphore value is currently 1
   * we could make it past the while loop so that one thread will
   * be allowed to proceed but another thread may also enter and it
   * will also be allowed to proceed if the first thread has not
   * yet reached the step where it decrements the semaphore */
  __sthread_lock();
  
  /* Multiple threads may calll semaphore_wait at the same time so
   * ensure that we block all of them through a while loop */
  while (semp->value == 0) {
    queue_add(semp, sthread_current());

    /* If the semaphore value is zero, we want to block the current
     * thread so that it does not busy wait until the sempahore
     * value is above zero, which would waste resources unnecessarily.
     * Instead, blocking the thread allows for a passive wait so that
     * if the thread is later unblocked it will just proceed from
     * when it was blocked and continue operations, mimicking a 
     * simple wait, but with better efficiency */
    sthread_block();
  }

  /* If a thread is unblocked, it will resume execution here so
   * lock the thread again to ensure that the wait operation is
   * atomic */
  __sthread_lock();

  /* Decrement the semaphore value as it is used by another thread */
  semp->value--;

  /* Unlock the thread to allow other threads to access sempahore_wait
   * as this thread has finished completing all critical operations */
  __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {
  /* Lock the thread to make sure that the function is not interrupted
   * in between. This could cause problems where the entire unblocking
   * operation is not completed before an interrupt, causing multiple
   * or no blocking operations to occur, when one should
   */
  __sthread_lock();

  /* Incrementing the semaphore signals that more of the resource
   * is available */
  semp->value ++;
  
  if (!queue_empty(semp)) {
    Thread *next = queue_take(semp);

    /* If there are blocked threads waiting in the queue, then unblock
     * the first one that entered. We must unblock that thread so
     * that it can proceed from the point where it was blocked to
     * continue what it was doing, and another thread gets a chance to
     * perform its necessary actions */
    sthread_unblock(next);
  }
  /* Unlock the thread to allow other threads to use semaphore_signal
   * now that this thread has completed the critical section, as no
   * more operations can be interrupted
   */
  __sthread_unlock();
}

