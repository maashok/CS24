#include "accum.h"
#include <stdlib.h>
#include "cmp_swap.h"


/* Adds "value" to *accum, returning the new total that is held in *accum as
 * a result of this particular addition (i.e. irrespective of other concurrent
 * additions that may be occurring).
 *
 * This function should be thread-safe, allowing calls from multiple threads
 * without generating spurious results.
 */
uint32_t add_to_accum(uint32_t *accum, uint32_t value) {
    /* Keeps trying until this process can perform without
     * being interrupted
     */
      while (1) {
            /* Stores what value the accumulator should store */
            int temp = *accum + value;
            /* If the value of the accumulator has not changed
             * by now, then move the sum into the accumulator
             * and exit the loop.
             * If the value of the accumulator has changed due
             * to a concurrent addition, then the current value
             * of the accumulator won't equal the sum we calculated
             * minus the value we added in, in which case we repeat
             * the loop
             */
            if(compare_and_swap(accum, temp - value, temp))
                break;
      }
      /* If the process gets here, it means the value of accum
       * is correct, so return it
       */
      return *accum;
}

