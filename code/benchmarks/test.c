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
    // dequeue(thread1);  // Dequeue is not working correctly
    // enqueue(thread3);  
    // printQ();
    // destroyQ();

    
    //--------------------------------------------------------------------------------------
   
    // WORKER_CREATE TESTING
    // createQueue();
    // init_timer();
    worker_t wid1;
    worker_t ret1 = worker_create(&wid1, NULL, &fibonacchi, NULL);

    // worker_t wid2;
    // worker_t ret2 = worker_create(&wid2, NULL, &fibonacchi, NULL);

    while (1) {
        // if (count == 2){
        //     exit(0);
        // }
    }
    //destroyQ();

    // Join the threads when done
/*    for (int i = 0; i < thread_num; ++i){
        int ret = worker_join(thread[i].tcb->wid, NULL);
        printf("join_status[%d]: %d\n", i, ret);
    }
*/

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
