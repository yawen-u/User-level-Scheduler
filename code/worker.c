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
#define INTERVAL 5            /* number of milliseconds to go off */


int numWorkers = 0;
bool first_invoke = true;


Queue* run_q;
bool join;
void *(*func)(void*);
void * arg;

ucontext_t main_context;
bool in_main = true;

wthread* scheduler;
wthread* current_worker;
wthread* join_worker;

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
        // setcontext( &(scheduler->tcb->context) );
        scheduler->tcb->status = RUNNING;

        printf("Scheduler context made;\n");
        return;
}

void init_timer() {
    
    // Initialize timer
    if (signal(SIGALRM, (void (*)(int))schedule) == SIG_ERR) {
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
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arguments) {

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


        current_worker->tcb->wid = numWorkers++;
        current_worker->tcb->context.uc_link = NULL;
        current_worker->tcb->stack = malloc(STACK_SIZE);
        current_worker->tcb->context.uc_stack.ss_sp = current_worker->tcb->stack;
        current_worker->tcb->context.uc_stack.ss_size = STACK_SIZE;
        current_worker->tcb->context.uc_stack.ss_flags = 0;
        current_worker->function = function;
        current_worker->arg = arguments;
        //current_worker->tcb.priority;

        if (current_worker->tcb->stack == NULL){
                perror("Failed to allocate stack");
                exit(1);
        }

        makecontext(&(current_worker->tcb->context),(void *)&execute,1, current_worker);

        // after everything is set, push this thread into run queue and make it ready for the execution
        enqueue(current_worker);
        current_worker->tcb->status = READY;

        return current_worker->tcb->wid;
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
      
        if (value_ptr != NULL){
                printf("exiting worker\n");
                // free(temp->tcb->stack);
                // free(temp->tcb);
                // free(temp);
        }

        return;
};

//------------------------------------------------------------------------------------------------------


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {

        printf("ENTER WORKER JOIN:\n");
        
        // - wait for a specific thread to terminate
        // - check if the thread already existed
        join = false;
        wthread* ptr = run_q->front;
        while (ptr != NULL) {
                if (ptr->tcb->wid == thread && ptr->tcb->status != TERMINATED) { // thread found
                        join = true; 
                        join_worker = ptr;

                        if (in_main){
                                printf("Main join: %d\n", ptr->tcb->wid);
                                in_main = false;
                               
                                swapcontext(&(scheduler->tcb->context), &(join_worker->tcb->context));
                        }

                        else { 
                                printf("Worker join: %d \n", ptr->tcb->wid);
                                in_main = true;
                                current_worker->tcb->status = WAIT;
                                swapcontext(&(current_worker->tcb->context), &(join_worker->tcb->context));
                        }

                        return 0;
                }
                ptr = ptr->next;
        }

        if (!join) {
                return 1;
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

        printf("Timer went off - Switch to scheduler context.\n");

        // Check first whether we are in the main context or running a worker
        if (in_main){

                printf("In main: %d\n", current_worker->tcb->wid);
                in_main = false;

                // FIFO
                printQ();
                if (run_q->front == NULL) {
                        printf("All finished.\n");
                        destroyQ();
                        exit(0);
                }

                // if current worker is done running, exit the terminated worker then run the next worker in queue
                if (current_worker->tcb->status == TERMINATED) {
                        printf("exit worker %d; ", current_worker->tcb->wid);
                        numWorkers--;
                } 

                // if there is a worker to join
                if (join) {
                        if (join_worker->tcb->status == TERMINATED) {
                                // if join_worker is done, remove it from the queue;
                                removeFQ(join_worker->tcb->wid);
                                join = false;

                                // All done with the joinning thread - select next to run
                                current_worker = dequeue(run_q);
                                current_worker->tcb->status = RUNNING;
                                printf("running worker %d\n", current_worker->tcb->wid);
                                swapcontext(&(scheduler->tcb->context), &(current_worker->tcb->context));

                        } else {
                                // else - keep wait for it to terminate
                                swapcontext(&(scheduler->tcb->context), &(join_worker->tcb->context));
                        }
                }
                
                // no worker to wait for
                else {
                        // select next worker in queue
                        current_worker = dequeue(run_q);
                        current_worker->tcb->status = RUNNING;
                        printf("running worker %d\n", current_worker->tcb->wid);
                       
                        swapcontext(&(scheduler->tcb->context), &(current_worker->tcb->context));
                }

        }

        // In worker thread - swap back to scheduler context
        else { 
                printf("In worker: %d \n", current_worker->tcb->wid);
                in_main = true;

                // Only enqueue workers who are not done running
                if (current_worker->tcb->status != TERMINATED && current_worker != join_worker) {
                        enqueue(current_worker);
                        current_worker->tcb->status = READY;
                }

                swapcontext(&(current_worker->tcb->context), &(scheduler->tcb->context));
        }
       

       /*free(scheduler->tcb->stack);
       free(scheduler->tcb);
       free(scheduler);*/



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

        if(numWorkers == run_q->capacity) {
                perror("run_queue is Full\n");
                exit(1);
        } else {
                worker->next = NULL;
                if (run_q->rear != NULL) {
                        run_q->rear->next = worker;
                }
                run_q->rear = worker;
                if(run_q->front == NULL) {
                        run_q->front = worker;  
                }
        }
}

wthread* dequeue() {
        
        wthread* curr = NULL;

        if(run_q == NULL) {
                perror("No more worker to dequeue.\n");
                exit(1);
        } else {
                curr = run_q->front;
                run_q->front = run_q->front->next;

                // In case the front becomes NULL
                if (run_q->front == NULL){
                        run_q->rear = NULL; // Also update the rear
                }
        }
        return curr;
}

// Remove a worker from the Q
void removeFQ(worker_t worker) {

        if (run_q->front == NULL) {
                perror("Empty Queue;\n");
                exit(1);
        }

        wthread* prev = NULL;
        wthread* temp = run_q->front;

        while (temp != NULL){
                if (temp->tcb->wid == worker) {
                        prev->next = temp->next;
                }
                prev = temp;
                temp = temp->next;
        }

        // free(temp->tcb);
        // free(temp);
}

void printQ(){
        if (run_q->front == NULL) {
                printf("Empty Queue;\n");
                return;
        }

        wthread* temp = run_q->front;

        while (temp != NULL) {
                printf("worker-id: %d; Status: %d\n", temp->tcb->wid, temp->tcb->status);
                temp = temp->next;
        }

}

void destroyQ() {
        wthread* curr = run_q->front;
        while (curr != NULL) {
                wthread* temp = curr->next;
                free(curr->tcb->stack);
                free(curr->tcb);
                free(curr);
                curr = temp;
        }
        free(run_q);
}


//------------------------------------------------------------------------------------------------------
// Helper functions

void execute(wthread* worker){

        printf("ENTER EXECUTE.\n");

        in_main = false;

        func = worker->function;
        arg = worker->arg;
        func(arg);

        printf("function done;\n");

        worker->tcb->status = TERMINATED;

        swapcontext( &(worker->tcb->context), &(scheduler->tcb->context));
}