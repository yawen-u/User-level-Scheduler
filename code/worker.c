// File:        worker.c

// List all group member's name: Yawen Xu, Tomás Aquino
// username of iLab: yx251, tma105
// iLab Server: ilab 4
//------------------------------------------------------------------------------------------------------

#include "worker.h"

// INITAILIZE ALL YOUR VARIABLES HERE
#define STACK_SIZE SIGSTKSZ
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define WAIT 3
#define TERMINATED 4
#define MAX_WORKERS 20
#define INTERVAL 500            /* number of milliseconds to go off */


int numWorkers = 0;
bool first_invoke = true;


Queue* run_q;
int exitedThreads[MAX_WORKERS];  // threads that have already exited
int exitId = 0;

ucontext_t main_context;

wthread* scheduler;
wthread* current_worker;

struct itimerval it_val;        /* for setting itimer */

//------------------------------------------------------------------------------------------------------
// Library Initialization

void initialize(){

        first_invoke = false; // We only initialize the first time

        createQueue();

        init_scheduler();

        init_timer();

}


void init_scheduler() {
        
        
        if( (scheduler = malloc(sizeof(wthread)) ) == NULL ) {
                printf("Could Not allocate Memory to wthread\n"); 
                exit(1);
        }
        if( (scheduler->tcb = malloc(sizeof(tcb)) ) == NULL ) {
                printf("Could Not allocate Memory to tcb\n"); 
                exit(1);
        }

        // - create and initialize the context of this worker thread
        if (getcontext( &(scheduler->tcb->context) ) < 0){
                perror("getcontext");
                exit(1);
        }
        scheduler->tcb->wid = numWorkers++;
        scheduler->tcb->context.uc_link = NULL;
        scheduler->tcb->stack = malloc(STACK_SIZE);
        scheduler->tcb->context.uc_stack.ss_sp = scheduler->tcb->stack;
        scheduler->tcb->context.uc_stack.ss_size = STACK_SIZE;
        scheduler->tcb->context.uc_stack.ss_flags = 0;
      
        if (scheduler->tcb->stack == NULL){
                perror("Failed to allocate stack");
                exit(1);
        }
        makecontext( &(scheduler->tcb->context), NULL, 0);
        setcontext( &(scheduler->tcb->context) );
        scheduler->tcb->status = RUNNING;
        return;
}

void init_timer() {
    
    // Initialize timer
    if (signal(SIGALRM, (void (*)(int)) DoStuff) == SIG_ERR) {

        printf("Unable to catch SIGALRM");
        exit(1);
    }

    it_val.it_value.tv_sec =     INTERVAL/1000;
    it_val.it_value.tv_usec =    (INTERVAL*1000) % 1000000;       
    it_val.it_interval = it_val.it_value;

    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {

        printf("error calling setitimer()");
        exit(1);
    }

}


//------------------------------------------------------------------------------------------------------
/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {

        if (first_invoke) {
                initialize();
        }

        // Check if the number of workers is full
        if (numWorkers == MAX_WORKERS){
                return -1;
        }

        // - create Thread Control Block (TCB)
        if( (current_worker = malloc(sizeof(wthread)) ) == NULL ) {
                printf("Could Not allocate Memory to wthread\n"); 
                exit(1);
        }
        //current_worker->function = function;

        // - create Thread Control Block (TCB)
        if( (current_worker->tcb = malloc(sizeof(tcb)) ) == NULL ) {
                printf("Could Not allocate Memory to tcb\n"); 
                exit(1);
        }

        // - create and initialize the context of this worker thread
        if (getcontext( &(current_worker->tcb->context) ) < 0){
                perror("getcontext");
                exit(1);
        }


        current_worker->tcb->wid = ++numWorkers;
        current_worker->tcb->context.uc_link = NULL;
        current_worker->tcb->stack = malloc(STACK_SIZE);
        current_worker->tcb->context.uc_stack.ss_sp = current_worker->tcb->stack;
        current_worker->tcb->context.uc_stack.ss_size = STACK_SIZE;
        current_worker->tcb->context.uc_stack.ss_flags = 0;
        //current_worker->tcb.priority;

        if (current_worker->tcb->stack == NULL){
                perror("Failed to allocate stack");
                exit(1);
        }
        makecontext( &(current_worker->tcb->context), (void *)function, 0);
        setcontext( &(current_worker->tcb->context) );

        // after everything is set, push this thread into run queue and make it ready for the execution
        enqueue(current_worker);
        current_worker->tcb->status = READY;

        return current_worker->tcb->wid;
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
        
        // - change worker thread's state from Running to Ready
        if (current_worker->tcb->status == RUNNING) {
                current_worker->tcb->status = READY;
        } else {
                printf("YIELDING ERROR: current thread is not running\n");
        }

        // - put the current worker thread back to a runqueue and choose the next worker thread to run
        enqueue(current_worker);
       
        // - save context of this thread to its thread control block; switch from thread context to scheduler context
        worker_exit(current_worker);
        swapcontext( &(current_worker->tcb->context), &(scheduler->tcb->context) ); 
        
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* terminate a thread */
void worker_exit(void *value_ptr) {

        // - de-allocate any dynamic memory created when starting this thread
        free(current_worker->tcb->stack);
        free(current_worker->tcb);
        free(current_worker);

};

//------------------------------------------------------------------------------------------------------


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
        
        // - wait for a specific thread to terminate
        // - check if the thread already existed
        bool join = true;
        for (int i = 0; i < MAX_WORKERS; i++) {
                if (thread == exitedThreads[i]) {
                        join = false;
                        printf("thead already existed.\n");
                        return 1;
                }
        }

        if (join) {
                // block the current thread
                current_worker->tcb->status = WAIT;

                // find the specific thread in queue
                wthread* temp_worker = run_q->front;
                while (temp_worker->next != NULL) {

                        if (current_worker->tcb->wid == thread) { // thread found
                                // saves the current context then swaps it out for the thread context.
                                swapcontext(&(current_worker->tcb->context), &(temp_worker->tcb->context));

                                // wait for termination
                                while (1) {
                                        if (temp_worker->tcb->status == TERMINATED) {
                                                exitedThreads[exitId] = temp_worker->tcb->wid;
                                                free(temp_worker->tcb->stack);
                                                break;
                                        } 
                                }
                                setcontext(&scheduler->tcb->context);
                                return 0;
                        } else {
                                temp_worker = temp_worker->next;
                        }

                }

        }
  
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {

        //- initialize data structures for this mutex

        return 0;
};

//------------------------------------------------------------------------------------------------------

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
        // - release mutex and make it available again. 
        // - put threads in block list to run queue 
        // so that they could compete for mutex later.

        // YOUR CODE HERE
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
        // - de-allocate dynamic memory created in worker_mutex_init

        return 0;
};

//------------------------------------------------------------------------------------------------------

/* scheduler */
static void schedule() {

        // - every time a timer interrupt occurs, your worker thread library 
        // should be contexted switched from a thread context to this 
        // schedule() function

       // FIFO
       current_worker = dequeue(run_q);
       current_worker->tcb->status = RUNNING;
       printQ();
       swapcontext(&(scheduler->tcb->context), &(current_worker->tcb->context));

       free(scheduler->tcb->stack);
       free(scheduler->tcb);
       free(scheduler);


        // - invoke scheduling algorithms according to the policy (RR or MLFQ)
        // if (sched == RR)
        //         sched_rr();
        // else if (sched == MLFQ)
        //         sched_mlfq();


// - schedule policy
#ifndef MLFQ
        // Choose RR
#else 
        // Choose MLFQ
#endif

}

//------------------------------------------------------------------------------------------------------

/* Round-robin (RR) scheduling algorithm */
static void sched_rr() {
        // - your own implementation of RR
        // (feel free to modify arguments and return types)

        // YOUR CODE HERE
}

//------------------------------------------------------------------------------------------------------

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
        // - your own implementation of MLFQ
        // (feel free to modify arguments and return types)

        // YOUR CODE HERE
}

//------------------------------------------------------------------------------------------------------

// Queue Functions
void createQueue() {

        run_q = (Queue *) malloc( sizeof(Queue) );
        run_q->capacity = MAX_WORKERS;
        run_q->front = run_q->rear = NULL;
}

void enqueue(wthread* worker) {

        if(numWorkers >= run_q->capacity) {
                perror("run_queue is Full\n");
                exit(1);
        } else {
                worker->next = NULL;

                // If the Queue is empty
                if (run_q->rear == NULL){

                        run_q->front = run_q->rear = worker;
                        return;
                }

                // Otherwise add at the rear
                run_q->rear->next = worker;
                run_q->rear = worker;
        }
}

wthread* dequeue(Queue *Q) {
        
        wthread* curr = NULL;

        if(numWorkers == 0) {
                perror("Queue is Empty\n");
                exit(1);
        } else {
                curr = Q->front;
                numWorkers--;
                Q->front = Q->front->next;

                // In case the front becomes NULL
                if (Q->front == NULL){
                        Q->rear = NULL; // Also update the rear
                }
        }
        
        return curr;
}

void printQ(){
        if (run_q->front == NULL) {
                printf("Empty Queue;\n");
                return;
        }

        wthread* temp = run_q->front;

        if (run_q->front == run_q->rear) { //only one worker
                printf("worker id: %d; Status: %d\n", temp->tcb->wid, temp->tcb->status);
        }
        while (temp != NULL) {
                printf("worker-id: %d; Status: %d\n", temp->tcb->wid, temp->tcb->status);
                temp = temp->next;
        }

}

void destroyQ() {
        wthread* curr = run_q->front;
        while (curr != NULL) {
                wthread* temp = curr->next;
                free(curr->tcb);
                free(curr);
                curr = temp;
        }
}


//------------------------------------------------------------------------------------------------------

void DoStuff(void) {

  printf("Timer went off.\n");

}