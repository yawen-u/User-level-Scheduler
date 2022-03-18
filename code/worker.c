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
bool first_run = true;
bool pato = true;

Queue* run_q;
Queue* join_q;
Queue* lock_q;

bool join;
unsigned int join_count = 0;
unsigned int lid_count = 0;

void *(*func)(void*);
void * arg;

ucontext_t main_context;

wthread* scheduler;
wthread* current_worker;
wthread* prev_worker;
wthread* join_worker;


wthread* special_case;
bool update_special = true;
bool done = false;

struct itimerval it_val;        /* for setting itimer */
unsigned int timeout = 0;

//------------------------------------------------------------------------------------------------------
// Library Initialization

void initialize(){

        first_invoke = false; // We only initialize the first time

        createQueue(&run_q);

        init_main_worker();

        init_scheduler();

        init_timer();

}

void init_main_worker(){

        // - create and initialize the context of this worker thread
        if (getcontext( &(main_context) ) < 0){
                perror("getcontext");
                exit(1);
        }

        return;
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
        makecontext( &(scheduler->tcb->context), (void *)&schedule, 0);
        // setcontext( &(scheduler->tcb->context) );
        scheduler->tcb->status = RUNNING;

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
        enqueue(&run_q, current_worker);
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
        enqueue(&run_q, current_worker);
       
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
                        join_count++; 
                        join_worker = ptr;
                        current_worker = join_worker;
                        swapcontext(&(scheduler->tcb->context), &(join_worker->tcb->context));

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

        if (mutexattr == NULL) {
                //PTHREAD_MUTEX_DEFAULT : permits a thread to lock many times;
        }

        if ((mutex = malloc(sizeof(worker_mutex_t))) == NULL) {
                perror("Unable to malloc for mutex.\n");
                return -1;
        }

        //createQueue(&lock_q);
        mutex->lid = lid_count++;
        mutex->lock = 0;
        mutex->count = 0;

        return 0;
};

//------------------------------------------------------------------------------------------------------

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        if (mutex->owner == NULL) {
                mutex->owner = current_worker;
                mutex->count++;
                printf("mutex count: %d;\n", mutex->count);
        }

        // wait for the mutex to become avaliable again
        while (atomic_flag_test_and_set(&(mutex->lock)) == 1) {
                // current_worker->tcb->status = BLOCKED;
                // printf("worker is waiting..\n");
        } // spin-wait

        // enter critical section
        printf("enter critical section\n");

        // if (mutex == NULL) { // acquiring mutex failed
        //         printf("Acquiring Mutex Failed\n");
        //         current_worker->tcb->status = BLOCKED;
        //         // enqueue(lock_q);
        //         schedule();
        // }

        schedule();
        
        return 0;
};

//------------------------------------------------------------------------------------------------------

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
        // - release mutex and make it available again. 
        // - put threads in block list to run queue 
        // so that they could compete for mutex later.

        mutex->count--;
        mutex->lock = 0;  // the mutex is availiable again

        return 0;
};

//------------------------------------------------------------------------------------------------------

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {

        // - de-allocate dynamic memory created in worker_mutex_init
        free(mutex);
        // destroyQ(&lock_q);

        return 0;
};

//------------------------------------------------------------------------------------------------------

/* scheduler */
static void schedule() {

        // - every time a timer interrupt occurs, your worker thread library 
        // should be contexted switched from a thread context to this 
        // schedule() function

        if (current_worker->tcb->status == TERMINATED && run_q->front == NULL) {
                printf("LAST WORKER TERMINATED.\n");
                numWorkers--;
                destroyQ(&run_q);
                exit(0);
        }

        // First run case
        if (first_run){

                first_run = false;
                prev_worker = NULL;
                current_worker = dequeue(&run_q);
                swapcontext(&(scheduler->tcb->context), &(current_worker->tcb->context));
        }

        // General case
        prev_worker = current_worker;
        current_worker = dequeue(&run_q);
        // printf("prev worker : %d; current worker : %d\n", prev_worker->tcb->wid, current_worker->tcb->wid);
       
        // Only enqueue workers who are not done running
        if (prev_worker->tcb->status != TERMINATED && prev_worker != join_worker) {
                enqueue(&run_q, prev_worker);
                prev_worker->tcb->status = READY;
        }

        // if current worker is done running, exit the terminated worker then run the next worker in queue
        else if (prev_worker->tcb->status == TERMINATED){
                printf("worker %d exited\n", prev_worker->tcb->wid);
                numWorkers--;
        }

        swapcontext(&(prev_worker->tcb->context), &(current_worker->tcb->context));
  


        // if (join) {
                
        //         if (join_worker->tcb->status == TERMINATED) {

        //                 // if join_worker is done, remove it from the queue;
        //                 removeFQ(&run_q, join_worker->tcb->wid);
        //                 join_count--;
        //                 printQ(&run_q);
        //                 join = false;

        //                 // All done with the joinning thread - select next to run
        //                 current_worker = dequeue(&run_q);

        //                 // Check if BLOCKED
        //                 while (current_worker->tcb->status == BLOCKED){

        //                         enqueue(&run_q, current_worker);
        //                         current_worker = dequeue(&run_q);
        //                 }

        //                 current_worker->tcb->status = RUNNING;
        //                 printf("running worker %d\n", current_worker->tcb->wid);
        //                 swapcontext(&(scheduler->tcb->context), &(current_worker->tcb->context));
        //         } 
        //         else {

        //                 // else - keep wait for it to terminate
        //                 swapcontext(&(scheduler->tcb->context), &(join_worker->tcb->context));
        //         }
        // }
                



                // // Check if BLOCKED
                // while (current_worker->tcb->status == BLOCKED){

                //         enqueue(&run_q, current_worker);
                //         current_worker = dequeue(&run_q);
                // }
 

       

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

void createQueue(Queue** Q) {

        (*Q) = (Queue *) malloc( sizeof(Queue) );
        (*Q)->capacity = MAX_WORKERS;
        (*Q)->front = (*Q)->rear = NULL;
       
}

void enqueue(Queue** Q, wthread* worker) {

        if(numWorkers == (*Q)->capacity) {
                perror("run_queue is Full\n");
                exit(1);
        } else {
                worker->next = NULL;
                if ((*Q)->rear != NULL) {
                        (*Q)->rear->next = worker;
                }
                (*Q)->rear = worker;
                if((*Q)->front == NULL) {
                        (*Q)->front = worker;  
                }
        }
}

wthread* dequeue(Queue** Q) {
        
        wthread* curr = NULL;

        if((*Q) == NULL || (*Q)->front == NULL) {
                return NULL;
        } else {
                curr = (*Q)->front;
                (*Q)->front = (*Q)->front->next;

                // In case the front becomes NULL
                if ((*Q)->front == NULL){
                        (*Q)->rear = NULL; // Also update the rear
                }
        }
        return curr;
}

// Remove a worker from the Q
void removeFQ(Queue** Q, worker_t worker) {

        if ((*Q)->front == NULL) {
                perror("Empty Queue;\n");
                exit(1);
        }

        wthread* prev = NULL;
        wthread* temp = (*Q)->front;

        while (temp != NULL){
                if (temp->tcb->wid == worker) {
                        if (temp == (*Q)->front) {
                                (*Q)->front = (*Q)->front->next;
                        } else if (temp == (*Q)->rear) {
                                prev->next = NULL;
                                (*Q)->rear = prev;
                        } else {
                                prev->next = temp->next;
                        }
                }
                prev = temp;
                temp = temp->next;
        }

        // free(temp->tcb);
        // free(temp);
}

void printQ(Queue** Q){
        if ((*Q)->front == NULL) {
                printf("Empty Queue;\n");
                return;
        }

        wthread* temp = (*Q)->front;

        while (temp != NULL) {
                printf("worker-id: %d; Status: %d\n", temp->tcb->wid, temp->tcb->status);
                temp = temp->next;
        }

}

void destroyQ(Queue** Q) {
        wthread* curr = (*Q)->front;
        while (curr != NULL) {
                wthread* temp = curr->next;
                free(curr->tcb->stack);
                free(curr->tcb);
                free(curr);
                curr = temp;
        }
        free((*Q));
}


//------------------------------------------------------------------------------------------------------
// Helper functions

void execute(wthread* worker){

        func = worker->function;
        arg = worker->arg;
        func(arg);


        worker->tcb->status = TERMINATED;

        if (run_q->front != NULL) {
                schedule();
        }
}