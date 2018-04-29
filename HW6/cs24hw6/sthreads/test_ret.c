#include <stdio.h>
#include "sthread.h"

/* Tests having multiple threads running */
static void threadFn(void *loop) {
  int i;
  /* Prints the loop index as many times as specified 
   * yields after each iteration */
  for (i = 0; i < *(int *)(loop); i++) {
    printf("The loop index %d \n", i);
    sthread_yield();
  }
  /* Thread will go into finished state and end execution */
}

/* Creates multiple threads that will loop different
 * number of times
 */
int main() {
  int loop1 = 20;
  /* This thread loops 20 times */
  sthread_create(threadFn, (void *)(&loop1));
  
  int loop2 = 49;
  /* This thread loops 49 times */
  sthread_create(threadFn, (void *)(&loop2));

  int loop3 = 39;
  /* This thread loops 39 times */
  sthread_create(threadFn, (void *)(&loop3));

  int loop4 = 10;
  /* This thread loops 10 times */
  sthread_create(threadFn, (void *)(&loop4));

  int loop5 = 60;
  /* This thread loops 60 times */
  sthread_create(threadFn, (void *)(&loop5));
  sthread_start();
}
