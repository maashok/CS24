#include "my_setjmp.h"
#include <stdio.h>
#include "c_except.h"
#include <ctype.h>

jmp_buf env;

/* This function tries to divide the first
 * argument by the second, restoring the state
 * to when setjmp was called if the second argument
 * is zero
 */
double divideNums(double a, double b) {
    if (b == 0)
      longjmp(env, DIVIDE_BY_ZERO);
    return a / b;
}

/* This function tests whether the DIVIDE_BY_ZERO
 * exception is thrown properly
 */
void divideByZeroException() {
  double x = 0.0 ;
  double y = 1.0;
  double z = 3.4;

  printf("Trying to calculate 1.0 * 3.4 / 0.0\n");

  /* Tries to calculate y*z/x, first saving the state in env */
  int res = setjmp(env);
  /* If setjmp returns zero if there is no exception */
  if (res == 0) {
    printf("The quotient is %f\n", y * divideNums(z, x));
    printf("Test case 1 fails\n");
  }
  /* If longjmp is called again, setjmp will return a nonzero
   * value signifying an exception was thrown
   */
  else if (res == DIVIDE_BY_ZERO) {
    printf("The divisor was zero, test case 1 works\n");
  }
}

/* Test the return value of long_jmp when the return value 0 is passed in */
void testLongJump1() {
  /* Make sure longjmp is only called once */
  int doOnce = 0;
  int res = setjmp(env);
  if (doOnce == 0) {
    doOnce = 1;
    longjmp(env, 0);
  }
  /* If the return value is 1, or UNSPECIFIED_EXCEPTION, the test case 
   * works 
   */
  if (res == UNSPECIFIED_EXCEPTION)
    printf("Long jump with argument 0 returns %d. Test case 2 works\n", res);
  else
    printf("Long jump with argument 0 returns %d. Test case 2 fails\n", res);
}

/* Test the return value of long_jmp when a non - zero return value 
 * is passed in 
 */
void testLongJump2() {
  /* Make sure long_jmp is only called once */
  int doOnce = 0;
  int res = setjmp(env);
  /* Pass in 5 as the return value */
  int n = 5;
  if (doOnce == 0) {
    doOnce = 1;
    longjmp(env, n);
  }
  /* If the value returned from setjmp when longjmp is called is equal to the
   * argument passed in, test case works
   */
  if (n == res)
    printf("Long jump with argument 5 returns %d. Test case 3 works\n", res);
  else
    printf("Long jump with argument 5 returns %d. Test case 3 fails\n", res);
}

/* This function checks whether x is greater than y, going back to the
 * state when setjmp is called if there is a number parse error, 
 * if either of the numbers is part of the alphabet
 */
int parsing(int x, int y) {
  if (isalpha(x) || isalpha(y))
    longjmp(env, NUMBER_PARSE_ERROR);
  return x > y;
}

/* This function tests whether the NUMBER_PARSE_ERROR
 * exception is thrown correctly
 */
void testNumParseError() {
  /* Pass two characters into the comparison function */
  char x = 'x';
  char y = 'y';

  printf("Checking if x is greater than y\n");

  /* First save the state in env */
  int res = setjmp(env);
  /* setjmp will always return 0 */
  if (res == 0) {
    /* Try to compare x and y, where an exception might be thrown */
    printf("The result of x > y is %d", parsing(x, y));
    printf("Test case 4 fails\n");
  }
  /* If longjmp is called, causing setjmp to return a NUMBER_
   * PARSE_ERROR, test case works */
  else if (res == NUMBER_PARSE_ERROR) {
     printf("The comparisons was between letters, test case 4 works\n");
  }
}

/* Check whether the stack is being affected by the
 * setjmp and longjmp calls
 */
void testStackMixup() {
  /* Declare local values on either side
   * of the jmp_buf
   */
  double b = 3.33;
  int x = 5;
  jmp_buf env2;
  int y = 7;
  char a = 'a';

  /* Call setjmp and longjmp (only once) */
  int doOnce = 0;
  setjmp(env2);
  if (doOnce == 0) {
    doOnce = 1;
    longjmp(env2, 0);
  }
  /* Check whether each of the local variables has
   * been affected
   */
  printf("The value of b used to be 3.33 and is now %f\n", b);
  printf("The value of x used to be 5 and is now %d\n", x);
  printf("The value of y used to be 7 and is now %d\n", y);
  printf("The value of a used to be 'a' and is now %c\n", a);

  /* If the values are still the same, this test case works */
  if (b == 3.33 && x == 5 && y == 7 && a == 'a')
    printf("Test case 5 works\n");
  else
    printf("Test case 5 fails\n");
}

/* This function performs 5 tests on set_jmp and long_jmp */
int main() {
  /* Tests whether divide by zero exception works */
  printf("1) Testing divide by zero error\n");
  divideByZeroException();

  /* Tests the return value of longjmp based on the argument passed in */
  printf("2) Testing if passing longjmp argument 0 returns 1\n");
  testLongJump1();

  printf("3) Testing if passing longjmp a non - zero argument returns \
the argument\n");
  testLongJump2();

  /* Test whether number parse error exception works */
  printf("4) Testing number parse error\n");
  testNumParseError();

  /* Tests whether stack is corrupted by setjmp/longjmp */
  printf("5) Testing stack corruption error\n");
  testStackMixup();
  


  return 0;
}
