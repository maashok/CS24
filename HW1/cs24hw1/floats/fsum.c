#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ffunc.h"


/* This function takes an array of single - precision floating point values,
 * and computes a sum in the order of the inputs.  Very simple.
 */
float fsum(FloatArray *floats) {
    float sum = 0;
    int i;

    for (i = 0; i < floats->count; i++)
        sum += floats->values[i];

    return sum;
}

/*
 * This function follows the principles of the Python fsum function
 * (Used code.activestate.com/recipes/393090/ to understand the
 * method of summing by partials).
 * 
 * This method works by keeping a list of partials of various summations,
 * where the sum of the partials equals the sum of the numbers you have
 * tried to sum so far. Basically, the rounded sum plus the round - off error
 * equals the actual sum of the two numbers. 
 * So, every time we try to add a number, it sums that number and all the 
 * partials we have. For each of these summations, it checks if there is 
 * any roundoff error, and if there is, it saves that roundoff error
 * as a partial. Finally, the sum (with roundoff) of each number and the
 * partials at that time is stored as the final partial.
 * 
 * In this way, we can add all the numbers in the list.
 * 
 * This is more accurate since it keeps track of round - off error for every
 * summation done, adding all these round - off errors at the end together and
 * adding it to the rounded sum, so that if the round - off errors can make a 
 * difference in the sum, they do at the end.
 */
float my_fsum(FloatArray *floats) {
    int i, j, m, k;
    /* Stores the number of partials that we have */
    int maxSize = 0;
    float partials[100000];
    for (i = 0; i < floats->count; i++) {
        /* Stores how many partials we have entered so far for this number
         * to sum
         */
        int count = 0;
        float sum;
        /* Starts off as the value we are adding to the partials.
         * Will contain the rounded sum with partials
         */
        float first = floats->values[i];
        /* For each of the partials we have */
        for (j = 0; j < maxSize; j++) {
            float second = partials[j];
            /* Store the rounded sum of the sum so far and the partial */
            sum = first + second;
            /* Calculate the round - off error in that summation (difference
             * between the number that was added and the number that 
             * should have been) 
             */
            float error = second - (sum - first);
            /* If there is some error, store that error as a partial */
            if (error != 0.0) {
                partials[count] = error;
                count++;
            }
            /* Note that none of the partials we need will ever be overwritten
             * since count will grow slower than or at the speed as j, so
             * we will have gotten the partial at the index already
             */
            
            /* Set the sum as the value we are adding the partials to */
            first = sum;
        }
        /* The last partial is the rounded sum */
        partials[count] = first;
        count++;
        /* Set other partials equal to zero so they don't affect final sum */
        for (k = count; k < 100000; k++) partials[k] = 0.0;
        /* Change the size of the partials depending on how many we used */
        maxSize = count;
    }
    /* Calculate the sum of the partials = sum of the list of numbers */
    float fullSum = 0.0;
    for (m = 0; m < 100000; m++) {
        fullSum += partials[m];
    }
    return fullSum;
}



int main() {
    FloatArray floats;
    float sum1, sum2, sum3, my_sum;

    load_floats(stdin, &floats);
    printf("Loaded %d floats from stdin.\n", floats.count);
    /* Compute a sum, in the order of input. */
    sum1 = fsum(&floats);

    /* Use my_fsum() to compute a sum of the values.  Ideally, your
     * summation function won't be affected by the order of the input floats.
     */
    my_sum = my_fsum(&floats);

    /* Compute a sum, in order of increasing magnitude. */
    sort_incmag(&floats);
    sum2 = fsum(&floats);

    /* Compute a sum, in order of decreasing magnitude. */
    sort_decmag(&floats);
    sum3 = fsum(&floats);

    /* %e prints the floating - point value in full precision,
     * using scientific notation.
     */
    printf("Sum computed in order of input:  %e\n", sum1);
    printf("Sum computed in order of increasing magnitude:  %e\n", sum2);
    printf("Sum computed in order of decreasing magnitude:  %e\n", sum3);
    
    printf("My sum:  %e\n", my_sum);
    

    return 0;
}

