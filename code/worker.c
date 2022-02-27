// File:        worker.c

// List all group member's name: Yawen Xu, Tomás Aquino
// username of iLab: yx251, tma105
// iLab Server: ilab 4

#include "worker.h"

// INITAILIZE ALL YOUR VARIABLES HERE
#define STACK_SIZE SIGSTKSZ
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define MAX_WORKERS 20;

int numWorkers = 0;
boolean first_invoke = true;
Queue run_q;
ucontext_t scheduler_context;

tcb *current_tcb;

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {

        // - create Thread Control Block (TCB)
        if( (current_tcb = malloc(sizeof(tcb)) ) == NULL ) {
                printf("\nCould Not allocate Memory to tcb"); 
                exit(0);
        }

        // - create and initialize the context of this worker thread
        if (getcontext(&current_tcb) < 0){
                perror("getcontext");
                exit(1);
        }

        current_tcb.wid = ++numWorkers;
        current_tcb.context.uc_link = NULL;
        current_tcb.stack = malloc(STACK_SIZE);
        current_tcb.context.uc_stack.ss_sp = current_tcb.stack;
        current_tcb.context.uc_stack.ss_size = STACK_SIZE;
        current_tcb.context.uc_stack.ss_flags = 0;
        //current_tcb.priority;

        if (current_tcb.stack == NULL){
                perror("Failed to allocate stack");
                exit(1);
        }
        makecontext(&current_tcb, (void *)function, 0);
        setcontext(&current_tcb);

        // after everything is set, push this thread into run queue and 
        // - make it ready for the execution.
        enqueue(run_q, tcb);
        current_tcb.status = READY;

        if (first_invoke) {
                first_invoke = false;
                current_tcb.context = scheduler_context;
        }

        return current_tcb.wid;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
        
        // - change worker thread's state from Running to Ready
        if (current_tcb.status == RUNNING) {
                current_tcb.status = READY;
        } else {
                printf("YIELDING ERROR: current thread is not running\n");
        }

        // - put the current worker thread back to a runqueue and choose the next worker thread to run
        enqueue(run_q, current_tcb);
       
        // - save context of this thread to its thread control block; switch from thread context to scheduler context
        swapcontext(&current_tcb.context, &scheduler_context); 
        
        return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {

        // - de-allocate any dynamic memory created when starting this thread
        free(current_tcb);
        free(current_tcb.stack);

};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
        
        // - wait for a specific thread to terminate

        // - de-allocate any dynamic memory created by the joining thread
  
        return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {

        //- initialize data structures for this mutex

        return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
        // - release mutex and make it available again. 
        // - put threads in block list to run queue 
        // so that they could compete for mutex later.

        // YOUR CODE HERE
        return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
        // - de-allocate dynamic memory created in worker_mutex_init

        return 0;
};

/* scheduler */
static void schedule() {
        // - every time a timer interrupt occurs, your worker thread library 
        // should be contexted switched from a thread context to this 
        // schedule() function

        // - invoke scheduling algorithms according to the policy (RR or MLFQ)

        // if (sched == RR)
        //              sched_rr();
        // else if (sched == MLFQ)
        //              sched_mlfq();

        // YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
        // Choose RR
#else 
        // Choose MLFQ
#endif

}

/* Round-robin (RR) scheduling algorithm */
static void sched_rr() {
        // - your own implementation of RR
        // (feel free to modify arguments and return types)

        // YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
        // - your own implementation of MLFQ
        // (feel free to modify arguments and return types)

        // YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

