// File:	worker_t.h

// List all group member's name: Yawen Xu, Tomás Aquino
// username of iLab: yx251, tma105
// iLab Server: ilab 4
//------------------------------------------------------------------------------------------------------

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

//------------------------------------------------------------------------------------------------------

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#include <signal.h>

//------------------------------------------------------------------------------------------------------

typedef uint worker_t;

typedef struct TCB {
	/* add important states in a thread control block */
	worker_t wid;// thread Id
	int status;// thread status
	ucontext_t context; // thread context
	void* stack;// thread stack
	int priority;// thread priority
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */

	// YOUR CODE HERE
} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)
typedef struct wthread {
	tcb* tcb; // Worker Thread TCB
	struct wthread* next;
} wthread;

typedef struct Queue {
	int capacity;
	wthread* front;
	wthread* rear;
} Queue;

//------------------------------------------------------------------------------------------------------

/* Function Declarations: */

// Library Initialization

void initialize();
void init_scheduler();
void init_timer();


//------------------------------------------------------------------------------------------------------

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);

/* Queue functions */
void createQueue();
void enqueue(wthread* worker);
wthread* dequeue(Queue *Q);
void printQ();


static void schedule();

void DoStuff(void);

//------------------------------------------------------------------------------------------------------

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
