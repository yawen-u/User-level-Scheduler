#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../worker.h"

void fiber1();
void* fibonacchi();
void squares();

/* A scratch program template on which to call and
 * test worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

worker_mutex_t* mutex;
int mail = 0;

void*  resource(int num){

    worker_mutex_lock(&mutex);
    mail = 0;
    for (int i=0; i < 5000; i++){
        mail++;
        printf("mail thread %d : %d\n", num, mail);
    }
    worker_mutex_unlock(&mutex);
}

void*  f1withparam(){
    
    for (int i=0; i < 5000; i++){
        printf("Thread 1:%d\n", i);
    }
}

void*  f2withparam(){
    for (int i=5000; i < 10000; i++){
        printf("Thread 2:%d\n", i);
    }
}

void*  f3withparam(){
    for (int i=10000; i < 15000; i++){
        printf("Thread 3:%d\n", i);
    }
}

void*  f4withparam(){
    for (int i=15000; i < 20000; i++){
        printf("Thread 4:%d\n", i);
    }
}

int main(int argc, char **argv) {

    // QUEUE TESTING

    // wthread* thread1 = (wthread*) malloc(sizeof(wthread));
    // thread1->tcb = malloc(sizeof(tcb));
    // thread1->tcb->wid = 1;
    // thread1->tcb->status = 1;

    // wthread* thread2 = (wthread*) malloc(sizeof(wthread));
    // thread2->tcb = malloc(sizeof(tcb));
    // thread2->tcb->wid = 2;
    // thread2->tcb->status = 1;

    // wthread* thread3 = (wthread*) malloc(sizeof(wthread));
    // thread3->tcb = malloc(sizeof(tcb));
    // thread3->tcb->wid = 3;
    // thread3->tcb->status = 1;

    // createQueue();
    // enqueue(thread1);
    // enqueue(thread2);
    // enqueue(thread3); 
    // wthread* deq1 = dequeue(); 
    // wthread* deq2 = dequeue();
    // wthread* deq3 = dequeue(); 
    // printQ();
    // destroyQ();

    
    //--------------------------------------------------------------------------------------
   
    // WORKER_CREATE TESTING
    
    worker_t wid1;
    worker_t ret1 = worker_create(&wid1, NULL, &f1withparam, NULL);

    worker_t wid2;
    worker_t ret2 = worker_create(&wid2, NULL, &f2withparam, NULL);

    worker_t wid3;
    worker_t ret3 = worker_create(&wid3, NULL, &f3withparam, NULL);

    worker_t wid4;
    worker_t ret4 = worker_create(&wid4, NULL, &f4withparam, NULL);

    // int retj1 = worker_join(ret2, NULL);

    // int retj2 = worker_join(ret3, NULL);

    // while (1) {
    //     sleep(2);
    //     printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    // }

    printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    


    // ---------------------------------------------------------------------------------
    // MUTEX TESTING
    
    // worker_mutex_init(&mutex, NULL);
    // worker_t wid1;
    // worker_t ret1 = worker_create(&wid1, NULL, &resource, 1);

    // worker_t wid2;
    // worker_t ret2 = worker_create(&wid2, NULL, &resource, 2);

    // worker_t wid3;
    // worker_t ret3 = worker_create(&wid3, NULL, &resource, 3);
    // worker_mutex_destroy(&mutex);


    // while (1) {
    //     sleep(2);
    //     printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    // }
    
	return 0;
}




void fiber1()
{
    int i;
    for ( i = 0; i < 5; ++ i )
    {
        printf( "Hey, I'm fiber #1: %d\n", i );
        //fiberYield();
    }
    return;
}

void* fibonacchi()
{
    int i;
    int fib[2] = { 0, 1 };
    
    sleep( 2 ); 
    printf( "fibonacchi(0) = 0\nfibonnachi(1) = 1\n" );
    for( i = 2; i < 15; ++ i )
    {
        int nextFib = fib[0] + fib[1];
        printf( "fibonacchi(%d) = %d\n", i, nextFib );
        fib[0] = fib[1];
        fib[1] = nextFib;
        //worker_yield();
    }
}

void squares()
{
    int i;
    
    /*sleep( 5 ); */
    for ( i = 0; i < 10; ++ i )
    {
        printf( "%d*%d = %d\n", i, i, i*i );
        //fiberYield();
    }
}
