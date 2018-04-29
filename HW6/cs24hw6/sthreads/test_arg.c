#include <stdio.h>
#include "sthread.h"

/* Tries to print something and the argument to the function */
static void printStuff(void *x) {
  printf("I am a new thread to print stuff!\n");
  int arg = *(int *)x;
  printf("The argument I got was %d \n", arg);
  sthread_yield();
  /* Thread will finish executing and end */
}

/* Does some arithmetic with the argument to the function */
static void arithmetic(void *x) {
  printf("I am a new thread to do arithmetic!\n");
  int arg = *(int *)x;
  /* Do some arithmetic to check manipulation of the argument */
  printf("The argument I got was %d \n", arg);
  printf("3 times the argument is %d \n", arg * 3);
  printf("That argument divided by 14 is %f \n", (double)arg / 14);
  sthread_yield();
  /* Thread will finish executing and end */
}


/* Creates a thread to run the printStuff function
 * Will test whether the argument is received correctly */
int main(int argc, char **argv) {
  int x = 5;
  /* Create a thread with argument 5 and start execution */
  sthread_create(printStuff, (void *)(&x));
  sthread_create(arithmetic, (void *)(&x));
  sthread_start();
  return 0;
}
