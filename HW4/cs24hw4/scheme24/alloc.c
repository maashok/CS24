#include "alloc.h"
#include "ptr_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/* ! Change to #define to output garbage-collector statistics. */
#define GC_STATS

/* !
 * Change to #undef to cause the garbage collector to only run when it has to.
 * This dramatically improves performance.
 *
 * However, while testing GC, it's easiest if you try it all the time, so that
 * the number of objects being manipulated is small and easy to understand.
 */
#define ALWAYS_GC


/* Change to #define for other verbose output. */
#undef VERBOSE


void free_value(Value *v);
void free_lambda(Lambda *f);
void free_environment(Environment *env);

void mark_environment(Environment *env);
void mark_value(Value *v);
void mark_lambda(Lambda *f);
void mark_eval_stack(PtrVector *pv);

/* !
 * A growable vector of pointers to all Value structs that are currently
 * allocated.
 */
static PtrVector allocated_values;


/* !
 * A growable vector of pointers to all Lambda structs that are currently
 * allocated.  Note that each Lambda struct will only have ONE Value struct 
 * that points to it.
 */
static PtrVector allocated_lambdas;


/* !
 * A growable vector of pointers to all Environment structs that are currently
 * allocated.
 */
static PtrVector allocated_environments;


#ifndef ALWAYS_GC

/* ! Starts at 1MB, and is doubled every time we can't stay within it. */
static long max_allocation_size = 1048576;

#endif


void init_alloc() {
    pv_init(&allocated_values);
    pv_init(&allocated_lambdas);
    pv_init(&allocated_environments);
}


/* !
 * This helper function prints some helpful details about the current 
 * allocation status of the program.
 */
void print_alloc_stats(FILE *f) {
    /*
    fprintf(f, "Allocation statistics:\n");
    fprintf(f, "\tAllocated environments:  %u\n", 
    allocated_environments.size);
    fprintf(f, "\tAllocated lambdas:  %u\n", allocated_lambdas.size);
    fprintf(f, "\tAllocated values:  %u\n", allocated_values.size);
    */

    fprintf(f, "%d vals \t%d lambdas \t%d envs\n", allocated_values.size,
        allocated_lambdas.size, allocated_environments.size);
}


/* !
 * This helper function returns the amount of memory currently being used by
 * garbage - collected objects.  It is NOT the total amount of memory being 
 * used by the interpreter!
 */ 
long allocation_size() {
    long size = 0;
    
    size += sizeof(Value) * allocated_values.size;
    size += sizeof(Lambda) * allocated_lambdas.size;
    size += sizeof(Value) * allocated_environments.size;
    
    return size;
}


/* !
 * This function heap - allocates a new Value struct, initializes it to be
 * empty, and then records the struct's pointer in the allocated_values 
 * vector.
 */
Value * alloc_value(void) {
    Value *v = malloc(sizeof(Value));
    memset(v, 0, sizeof(Value));

    pv_add_elem(&allocated_values, v);

    return v;
}


/* !
 * This function frees a heap - allocated Value struct.  Since a Value struct 
 * can represent several different kinds of values, the function looks at the
 * value's type tag to determine if additional memory needs to be freed for 
 * the value.
 *
 * Note:  It is assumed that the value's pointer has already been removed from
 *        the allocated_values vector!  If this is not the case, serious 
 *        errors will almost certainly occur.
 */
void free_value(Value *v) {
    assert(v != NULL);

    /*
     * If value refers to a lambda, we don't free it here!  Lambdas are freed
     * by the free_lambda() function, and that is called when cleaning up
     * unreachable objects.
     */

    if (v->type == T_String || v->type == T_Atom || v->type == T_Error)
        free(v->string_val);

    free(v);
}



/* !
 * This function heap - allocates a new Lambda struct, initializes it to be 
 * empty, and then records the struct's pointer in the allocated_lambdas 
 * vector.
 */
Lambda * alloc_lambda(void) {
    Lambda *f = malloc(sizeof(Lambda));
    memset(f, 0, sizeof(Lambda));

    pv_add_elem(&allocated_lambdas, f);

    return f;
}


/* !
 * This function frees a heap - allocated Lambda struct.
 *
 * Note:  It is assumed that the lambda's pointer has already been removed 
 *        from the allocated_labmdas vector!  If this is not the case, 
 *        serious errors will almost certainly occur.
 */
void free_lambda(Lambda *f) {
    assert(f != NULL);

    /* Lambdas typically reference lists of Value objects for the argument -
     * spec and the body, but we don't need to free these here because 
     * they are managed separately.
     */

    free(f);
}


/* !
 * This function heap - allocates a new Environment struct, initializes it 
 * to be empty, and then records the struct's pointer in the 
 * allocated_environments vector.
 */
Environment * alloc_environment(void) {
    Environment *env = malloc(sizeof(Environment));
    memset(env, 0, sizeof(Environment));

    pv_add_elem(&allocated_environments, env);

    return env;
}


/* !
 * This function frees a heap - allocated Environment struct.  The 
 * environment's bindings are also freed since they are owned by the 
 * environment, but the binding - values are not freed since they are 
 * externally managed.
 *
 * Note:  It is assumed that the environment's pointer has already been 
 *        removed from the allocated_environments vector!  If this is not 
 *        the case, serious errors will almost certainly occur.
 */
void free_environment(Environment *env) {
    int i;

    /* Free the bindings in the environment first. */
    for (i = 0; i < env->num_bindings; i++) {
        free(env->bindings[i].name);
        /* Don't free the value, since those are handled separately. */
    }
    free(env->bindings);

    /* Now free the environment object itself. */
    free(env);
}

/* !
 * This function marks all variables associated with the passed
 * in Environment struct, if they have not been marked already
 */
void mark_environment(Environment *env) {
  if (env->marked) return;

  Environment *curr = env;
  /* Go through the environments associated with the
   * one passed in iteratively
   */
  while (curr != NULL) {
    /* Mark this environment */
    curr->marked = 1;
    int i;
    /* Mark each of the bindings associated with the environment */
    for (i = 0; i < curr->num_bindings; i++) {
      mark_value(curr->bindings[i].value);
    }
    /* Move to the parent environment */
    curr = curr->parent_env;
  }
}

/* This function marks the value passed in as
 * well as other structs associated with it.
 */
void mark_value(Value *v) {
  if (v->marked) return;
  /* Mark the value */
  v->marked = 1;
  /* If the data is of type lambda, mark the lambda */
  if (v->type == T_Lambda) {
     mark_lambda(v->lambda_val);
  }
  /* If the data is of type ConsPair, mark both values
   * associated with the pair
   */
  else if(v->type == T_ConsPair) {
    mark_value(v->cons_val.p_car);
    mark_value(v->cons_val.p_cdr);
  }
}

/* This function marks a lambda and the values and
 * environment associated with it
 */
void mark_lambda(Lambda *f) {
  if (f->marked) return;
  f->marked = 1;

  /* If the function doesn't have a native implemenation
   * mark the arguments and body associated with it
   */
  if (f->native_impl != 1) {
    mark_value(f->arg_spec);
    mark_value(f->body);
  }
  /* Mark the parent environment of the lambda */
  mark_environment(f->parent_env);
}

/* This function marks all values and environments associated
 * with the eval stack
 */
void mark_eval_stack(PtrVector *pv) {
  int i, j;
  /* Each of the elements of the pointer vector is a pointer
   * to an evaluation context
   */
  for (i = 0; i < pv->size; i++) {
    EvaluationContext *ec = (EvaluationContext *) pv_get_elem(pv, i);
    
    /* As long as each of the attributes is not NULL, mark them */
    if (ec->current_env != NULL)
      mark_environment(ec->current_env);
    if (ec->expression != NULL)
      mark_value(ec->expression);
    if (ec->child_eval_result != NULL)
      mark_value(ec->child_eval_result);

    /* Go through each of the values in the local_vals and mark them */
    for (j = 0; j < ec->local_vals.size; j++) {
      Value **toMark = (Value **) pv_get_elem(&ec->local_vals, j);
      if (*toMark != NULL)
        mark_value(*toMark);
    }
  }
}

/* This function sweeps all unmarked values, freeing the memory
 * used by them and updating the list of allocated values
 */
void sweep_values() {
  Value *del;
  int i;
  /* Goes through the allocated values list and retrieves each value */
  for (i = 0; i < allocated_values.size; i++) {
    del = (Value *) pv_get_elem(&allocated_values, i);
    /* If the value is not marked, meaning it is not accessible
     * from the global environment or evaluation stack, it can
     * be freed
     */
    if (!del->marked) {
      free_value(del);
      pv_set_elem(&allocated_values, i, NULL);
    }
    /* If the value is marked, then unmark it so that the
     * same mark and sweep process can take place next time
     */
    else
      del->marked = 0;
  }
  pv_compact(&allocated_values);
}

/* This function sweeps all unmarked lambdas, freeing the memory
 * used by them and updating the list of allocated lambdas
 */
void sweep_lambdas() {
  Lambda *del;
  int i;
  /* Go through the list of allocated lambdas and retrieve each */
  for (i = 0; i < allocated_lambdas.size; i++) {
    del = (Lambda *) pv_get_elem(&allocated_lambdas, i);
    /* If the lambda is not marked, and is not accessible
     * from the global context, then free the memory for it
     */
    if (!del->marked) {
      free_lambda(del);
      pv_set_elem(&allocated_lambdas, i, NULL);
    }
    /* If it is marked, unmark it for the mark and sweep process
     * next time
     */
    else
      del->marked = 0;
  }
  pv_compact(&allocated_lambdas);
}

/* This function sweeps all the unmarked environments, freeing
 * the memory used by them
 */
void sweep_environments() {
  Environment *del;
  int i;
  /* Go through the list of environments and retrieve each one */
  for (i = 0; i < allocated_environments.size; i++) {
    del = (Environment *) pv_get_elem(&allocated_environments, i);
    /* If the environment is not marked, and is not accessible
     * from the global context, free the memory for it
     */
    if (!del->marked) {
      free_environment(del);
      pv_set_elem(&allocated_environments, i, NULL);
    }
    /* Otherwise unmark the environment, to mark/sweep next time */
    else
      del->marked = 0;
  }
  pv_compact(&allocated_environments);
}

/* !
 * This function performs the garbage collection for the Scheme interpreter.
 * It also contains code to track how many objects were collected on each run,
 * and also it can optionally be set to do GC when the total memory used grows
 * beyond a certain limit.
 */
void collect_garbage() {
    Environment *global_env;
    PtrStack *eval_stack;

#ifdef GC_STATS
    int vals_before, procs_before, envs_before;
    int vals_after, procs_after, envs_after;

    vals_before = allocated_values.size;
    procs_before = allocated_lambdas.size;
    envs_before = allocated_environments.size;
#endif

#ifndef ALWAYS_GC
    /* Don't perform garbage collection if we still have room to grow. */
    if (allocation_size() < max_allocation_size)
        return;
#endif

    global_env = get_global_environment();
    eval_stack = get_eval_stack();

    /* Start marking accessible elements from the global
     * environment and evaluation stack */
    mark_environment(global_env);
    mark_eval_stack(eval_stack);

    /* Then sweep all values, lambdas, and environments */
    sweep_values();
    sweep_lambdas();
    sweep_environments();

#ifndef ALWAYS_GC
    /* If we are still above the maximum allocation size, increase it. */
    if (allocation_size() > max_allocation_size) {
        max_allocation_size *= 2;

        printf("Increasing maximum allocation size to %ld bytes.\n",
            max_allocation_size);
    }
#endif
    
#ifdef GC_STATS
    vals_after = allocated_values.size;
    procs_after = allocated_lambdas.size;
    envs_after = allocated_environments.size;

    printf("GC Results:\n");
    printf("\tBefore: \t%d vals \t%d lambdas \t%d envs\n",
            vals_before, procs_before, envs_before);
    printf("\tAfter:  \t%d vals \t%d lambdas \t%d envs\n",
            vals_after, procs_after, envs_after);
    printf("\tChange: \t%d vals \t%d lambdas \t%d envs\n",
            vals_after - vals_before, procs_after - procs_before,
            envs_after - envs_before);
#endif
}

