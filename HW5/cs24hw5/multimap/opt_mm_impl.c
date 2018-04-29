#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multimap.h"


/* The size of the values array starts at 20 */
unsigned int startSize = 20;

/* The pool of all the nodes */
char **pool;

/* Stores which chunk of the pool we are on */
int poolIndex = 0;

/* We start out with 5 pools */
int numPools = 5;

/* Store where in the chunk we have allocated up to */
unsigned static int chunkIndex = 0;

/* The size of each chunk in the pool */
unsigned int poolSize = 2000;

/*============================================================================
 * TYPES
 *
 *   These types are defined in the implementation file so that they can
 *   be kept hidden to code outside this source file.  This is not for any
 *   security reason, but rather just so we can enforce that our testing
 *   programs are generic and don't have any access to implementation details.
 *==========================================================================*/

/* Represents a key and its associated values in the multimap, as well as
 * pointers to the left and right child nodes in the multimap. */
typedef struct multimap_node {
    /* The key-value that this multimap node represents. */
    int key;

    /* The values assoicated with the node */
    int *values;

    /* The current size of the values array */
    int currSize;
    /* The index of the values array we have added up to so far */
    int currIndex;
  
    /* The left child of the multimap node.  This will reference nodes that
     * hold keys that are strictly less than this node's key.
     */
    struct multimap_node *left_child;

    /* The right child of the multimap node.  This will reference nodes that
     * hold keys that are strictly greater than this node's key.
     */
    struct multimap_node *right_child;
} multimap_node;


/* The entry-point of the multimap data structure. */
struct multimap {
    multimap_node *root;
};


/*============================================================================
 * HELPER FUNCTION DECLARATIONS
 *
 *   Declarations of helper functions that are local to this module.  Again,
 *   these are not visible outside of this module.
 *==========================================================================*/

multimap_node * alloc_mm_node();

multimap_node * find_mm_node(multimap_node *root, int key,
                             int create_if_not_found);

void free_multimap_node(multimap_node *node);


/*============================================================================
 * FUNCTION IMPLEMENTATIONS
 *==========================================================================*/

/* Allocates a chunk of memory, and zeros out its contents so that we know 
 * what the initial value of everything will be.
 */
char *alloc_chunk() {
  /* If the pool hasn't been initialized, initialize it */
  if (pool == NULL) {
    pool = malloc(sizeof(char *) * numPools);
    poolIndex = 0;
  }
  /* If this pool has been used fully, allocate more memory */
  else if (poolIndex >= numPools) {
    numPools *= 2;
    pool = realloc(pool, sizeof(char *) * numPools);
  }
  /* Allocate a new chunk of memory and add it to the array of pools */
  char *chunk = malloc(poolSize);
  bzero(chunk, poolSize);
  pool[poolIndex] = chunk;
  chunkIndex = 0;
  return chunk;
}

/* Allocates a node from a chunk of memory if there is still space, otherwise
 * allocates a new chunk */
char *alloc_Space(int size, multimap_node *root) {
  char *toRet;
  /* If the current chunk or entire pool hasn't been initialized,
   * allocate memory for it */
  if (pool == NULL || pool[poolIndex] == NULL) {
    chunkIndex = size;
    toRet = alloc_chunk();
  }
  /* If we haven't filled the entire chunk, just 
   * allocate memory from it */
  if (chunkIndex <= poolSize - size) {
    toRet = pool[poolIndex] + chunkIndex;
    chunkIndex += size;
  }
  /* Otherwise allocate a new chunk */
  else {
    poolIndex++;
    toRet = alloc_chunk();
        chunkIndex = size;
  }
  return toRet;
}


/* This helper function searches for the multimap node that contains the
 * specified key.  If such a node doesn't exist, the function can initialize
 * a new node and add this into the structure, or it will simply return NULL.
 * The one exception is the root - if the root is NULL then the function will
 * return a new root node.
 */
multimap_node * find_mm_node(multimap_node *root, int key,
                             int create_if_not_found) {
    multimap_node *node = NULL;

    /* If the entire multimap is empty, the root will be NULL. */
    if (root == NULL) {
        if (create_if_not_found) {
            root = (multimap_node *)alloc_Space(sizeof(multimap_node), root);
            root->key = key;
        }
        return root;
    }

    /* Now we know the multimap has at least a root node, so start there. */
    node = root;

    while (1) {
      if (node->key == key) {
            break;
      }
        if (node->key > key) {   /* Follow left child */
            if (node->left_child == NULL && create_if_not_found) {
                /* No left child, but caller wants us to create a new node. */
                multimap_node *new = (multimap_node *)
                  alloc_Space(sizeof(multimap_node), root);
                new->key = key;
                node->left_child = new;
            }
            node = node->left_child;
            
        }
        else {                   /* Follow right child */
            if (node->right_child == NULL && create_if_not_found) {
                /* No right child, but caller wants us to create one. */
                multimap_node *new = (multimap_node *)
                  alloc_Space(sizeof(multimap_node), root);
                new->key = key;
                node->right_child = new;
            }
            node = node->right_child;
        }

        if (node == NULL)
            break;
    }
    return node;

 }


/* This helper function frees a multimap node, including its children and
 * value - list.
 */
void free_multimap_node(multimap_node *node) {
    if (node == NULL)
        return;

    /* Free the children first. */
    free_multimap_node(node->left_child);
    free_multimap_node(node->right_child);

    /* Free the list of values. */
    free(node->values);

#ifdef DEBUG_ZERO
    /* Clear out what we are about to free, to expose issues quickly. */
    bzero(node, sizeof(multimap_node));
#endif
}


/* Initialize a multimap data structure. */
multimap * init_multimap() {
    multimap *mm = malloc(sizeof(multimap *));
    mm->root = NULL;
    return mm;
}


/* Release all dynamically allocated memory associated with the multimap
 * data structure.
 */
void clear_multimap(multimap *mm) {
    assert(mm != NULL);
    /* Free the nodes */
    free_multimap_node(mm->root);
    /* Free the chunks of memory in the pool */
    int i;
    for (i = 0; i <= poolIndex; i++) {
      free(pool[poolIndex]);
      pool[poolIndex] = NULL;
    }
    /* Free the pool itself */
    free(pool);
    pool = NULL;
    /* Set the index back to 0, to start the pool over */
    poolIndex = 0;
    mm->root = NULL;
}


/* Adds the specified (key, value) pair to the multimap. */
void mm_add_value(multimap *mm, int key, int value) {
    multimap_node *node;

    assert(mm != NULL);

    /* Look up the node with the specified key.  Create if not found. */
    node = find_mm_node(mm->root, key, /* create */ 1);
    if (mm->root == NULL) {
        mm->root = node;
    }

    assert(node != NULL);
    assert(node->key == key);

    /* Add the new value to the multimap node. */

    /* If there are no values so far, allocate an array for them */
    if (node->values == NULL) {
       node->currSize = startSize;
       node->values = (int *)malloc(node->currSize * sizeof(int));
       node->currIndex = 0;
    }
    /* If more values are in the array than there is space for
     * reallocate more memory for the value */
    if (node->currIndex >= node->currSize) {
      int *newArray = (int *)realloc(node->values,
                                     node->currSize * 2 * sizeof(int));
      node->currSize *= 2;
      node->values = newArray;
    }
    
    node->values[node->currIndex] = value;
    node->currIndex++;
}


/* Returns nonzero if the multimap contains the specified key - value, zero
 * otherwise.
 */
int mm_contains_key(multimap *mm, int key) {
    return find_mm_node(mm->root, key, /* create */ 0) != NULL;
}


/* Returns nonzero if the multimap contains the specified (key, value) pair,
 * zero otherwise.
 */
int mm_contains_pair(multimap *mm, int key, int value) {
    multimap_node *node;
    int *curr;
    /* Find the node */
    node = find_mm_node(mm->root, key, /* create */ 0);
    /* If it wasn't there, then the pair doesn't exist */
    if (node == NULL)
        return 0;
    /* Search through the values for this pair */
    curr = node->values;
    int i = 0;
    while (i < node->currIndex) {
        if (curr[i] == value)
            return 1;
        i ++;
    }

    return 0;
}


/* This helper function is used by mm_traverse() to traverse every pair within
 * the multimap.
 */
void mm_traverse_helper(multimap_node *node, void (*f)(int key, int value)) {
    int *curr;

    if (node == NULL)
        return;

    /* Go through the values and apply the function to each pair */
    curr = node->values;
    int i = 0;
    while (i < node->currIndex) {
        f(node->key, curr[i]);
        i++;
    }
    /* Do the same to both children */
    mm_traverse_helper(node->left_child, f);


    mm_traverse_helper(node->right_child, f);
}


/* Performs an in - order traversal of the multimap, passing each (key, value)
 * pair to the specified function.
 */
void mm_traverse(multimap *mm, void (*f)(int key, int value)) {
    mm_traverse_helper(mm->root, f);
}

