/*============================================================================
 * Implementation of the FIFO page replacement policy.
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
    
    /* This is the first element of the queue of pages that are 
     * actually loaded. They are stored in FIFO order so it is 
     * easy to evict a victim from the front of the queue */
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
    fprintf(stderr, "Using FIFO eviction policy.\n\n");
    
    loaded = malloc(sizeof(loaded_pages_t));
    if (loaded) {
        loaded->max_resident = max_resident;
        loaded->num_loaded = 0;
        /* Initially there are no elements in the queue */
        loaded->first = NULL;
        loaded->last = NULL;
    }
    
    /* Return nonzero if initialization succeeded. */
    return (loaded != NULL);
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    pageQueue *temp;
    /* Go through all the elements of the queue and free
     * the memory for them */
    while (temp != NULL) {
        temp = loaded->first;
        loaded->first = temp->next;
        free(temp);
    }
    /* Free the memory for the loaded pages structure itself */
    free(loaded);
    loaded = NULL;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    assert(loaded->num_loaded < loaded->max_resident);

    /* If there are no elements in te queue, allocate
     * space for the first (which is also the last) element
     * and make the first element hold the page we are mapping in */    
    if (loaded->first == NULL) {
        loaded->first = malloc(sizeof(pageQueue));
        loaded->last = loaded->first;
        loaded->first->page = page;
        loaded->first->next = NULL;
    }
    else {
      /* Otherwise, if we have elements already, create a new
       * element with this page and make it the new last
       * element of the queue */
        pageQueue *temp = malloc(sizeof(pageQueue));
        temp->page = page;
        temp->next = NULL;
        loaded->last->next = temp;
        loaded->last = loaded->last->next;
    }
    /* Another page is now loaded */
    loaded->num_loaded++;
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    /* Do nothing since we do need to change anything at regular intervals! */
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  This is very simple since we are implementing a FIFO
 * policy
 */
page_t choose_and_evict_victim_page(void) {

    page_t victim;

    pageQueue *temp = loaded->first;
    /* Evict the first page in the queue. */
    victim = loaded->first->page;

    /* Shrink the collection of loaded pages, by making the previously
     * second element now the first
     */
    loaded->num_loaded--;
    loaded->first = loaded->first->next;

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    free(temp);
    
    return victim;

}

