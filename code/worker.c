// File:	worker.c

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
worker_t worker_list[MAX_WORKERS];

worker_t* run_q;

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {

	// - create Thread Control Block (TCB)
	tcb current_tcb;
	thread = (worker_t*)malloc(sizeof(worker_t));

	// - create and initialize the context of this worker thread
	if (getcontext(&current_tcb) < 0){
		perror("getcontext");
		exit(1);
	}

	current_tcb.wid = numWorkers;
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

	// after everything is set, push this thread into run queue and 
	// - make it ready for the execution.
	push(run_q, tcb);
	current_tcb.status = READY;
	numWorkers++;

	return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
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
	//		sched_rr();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

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

