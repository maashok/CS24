/*============================================================================
 * Implementation of the CLOCK/LRU page replacement policy.
 *
 * We don't mind if paging policies use malloc() and free(), just because it
 * keeps things simpler.  In real life, the pager would use the kernel memory
 * allocator to manage data structures, and it would endeavor to allocate and
 * release as little memory as possible to avoid making the kernel slow and
 * prone to memory fragmentation issues.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmpolicy.h"

/*============================================================================
 * "Page Queue" Data Structure
 *
 * This data structure stores a queue of all the pages currently loaded
 * in the virtual memory, so evicting a page is simply removing the first
 * element.
 */
typedef struct pages_queue_t {
    /* This element holds the address of the page */
    page_t page;

    /* Stores the address of the previous queue member to link them */
    struct pages_queue_t *prev;
  
    /* Stores the address of the next queue member to link them */
    struct pages_queue_t *next;
} pageQueue;

/*============================================================================
 * "Loaded Pages" Data Structure
 *
 * This data structure records all pages that are currently loaded in the
 * virtual memory, so that we can choose a random page to evict very easily.
 */

typedef struct loaded_pages_t {
    /* The maximum number of pages that can be resident in memory at once. */
    int max_resident;
    
    /* The number of pages that are currently loaded.  This can initially be
     * less than max_resident.
     */
    int num_loaded;
    
    /* This is the first element of the queue of pages that are actually 
     * loaded. They are stored in FIFO order so it is easy to evict a victim
     * from the front of the queue */
    pageQueue *first;

    /* This is the last element of the queue of pages that are actually loaded.
     * We add elements to the end of the queue */
    pageQueue *last;
} loaded_pages_t;


/*============================================================================
 * Policy Implementation
 */


/* The list of pages that are currently resident. */
static loaded_pages_t *loaded;


/* Initialize the policy.  Return nonzero for success, 0 for failure. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using CLOCK/LRU eviction policy.\n\n");
    
    loaded = malloc(sizeof(loaded_pages_t));
    if (loaded) {
        loaded->max_resident = max_resident;
        loaded->num_loaded = 0;
        /* Initially, there are no elements in the queue */
        loaded->first = NULL;
        loaded->last = NULL;
    }
    
    /* Return nonzero if initialization succeeded. */
    return (loaded != NULL);
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    pageQueue *temp;
    /* Go through the queue and free the memory allocated
     * for each element */
    while (temp != NULL) {
        temp = loaded->first;
        loaded->first = temp->next;
        free(temp);
    }
    /* Free up the memory from the loaded pages structure itself */
    free(loaded);
    loaded = NULL;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    assert(loaded->num_loaded < loaded->max_resident);

    /* If there are no elements in the queue, the new
     * page is placed in the first (which is also the last)
     * element. */
    if (loaded->first == NULL) {
        loaded->first = malloc(sizeof(pageQueue));
        loaded->first->page = page;
        loaded->first->next = NULL;
        loaded->first->prev = NULL;
        loaded->last = loaded->first;
    }
    else {
      /* Otherwise, a new element is allocated with this page
       * and placed at the back of the queue */
        pageQueue *temp = malloc(sizeof(pageQueue));
        temp->page = page;
        temp->next = NULL;
        loaded->last->next = temp;
        temp->prev = loaded->last;
        loaded->last = loaded->last->next;
    }
    /* Initially, the page is given no permissions, so that
     * we can know whenvever it is accessed */
    set_page_permission(page, PAGEPERM_NONE);
    loaded->num_loaded++;
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {

  /* We will go through all elements of the queue, starting
   * with the first */
    pageQueue *temp = loaded->first;
    pageQueue *storeNext;

    while (temp != NULL) {
      /* Since we will manipulate the queue, store the next element
       * to visit for the next iteration */
        storeNext = temp->next;

        /* If the page has been accessed we need to move it to the
         * back of the queue */
        if (is_page_accessed(temp->page)) {
           if (temp == loaded->first) {
             /* If there is only one element, we don't need
              * to move anything and we are done traversing the queue
              */
             if (loaded->first == loaded->last) {
               temp = NULL;
               continue;
             }
             /* Otherwise, if we are on the first element, just make
              * the second element the new first */
             loaded->first = temp->next;
             loaded->first->prev = NULL;
           }
           /* If we are not on the first element, directly link the 
            * previous element to the next */
           else {
               temp->prev->next = temp->next;
               if (temp->next != NULL)
                    temp->next->prev = temp->prev;
           }

           /* The last element will now become the current element */
           loaded->last->next = temp;
           temp->prev = loaded->last;
           temp->next = NULL;
           loaded->last = loaded->last->next;

           /* Set the page back up with no permissions and that it has
            * not been accessed so that we can detect the next access */
           clear_page_accessed(temp->page);
           set_page_permission(temp->page, PAGEPERM_NONE);
        }
        /* Move in our traversal to the next element */
        temp = storeNext;
    }
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  This is very simple since we are implementing a queue
 */
page_t choose_and_evict_victim_page(void) {
    page_t victim;


    /* Figure out which page to evict. */
    victim = loaded->first->page;
    /* Shrink the collection of loaded pages now, by making the previously
     * second element the first now. The evicted page was likely not accessed
     * recently since recently accessed pages were moved to the back of the
     * queue
     */
    loaded->num_loaded--;
    loaded->first = loaded->first->next;
    loaded->first->prev = NULL;

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}

