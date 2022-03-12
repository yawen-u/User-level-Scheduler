#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../worker.h"

void fiber1();
void fibonacchi();
void squares();

/* A scratch program template on which to call and
 * test worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
int main(int argc, char **argv) {

	/* Implement HERE */

    // Initialize the library
    // initialize_lib();

    

    // Create worker threads for testing
    // worker_t wid;
    // worker_t result;

    // result = worker_create(&wid, NULL, &squares, NULL);


    int thread_num = 100;
    wthread* thread = (wthread*) malloc(thread_num*sizeof(wthread));

    for (int i = 0; i < thread_num; ++i){
        int wid = worker_create(&(thread[i].tcb->wid), NULL, &fibonacchi, NULL);
        printf("wid[%d]: %d\n", i, thread[i]);
    }

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

void fibonacchi()
{
    int i;
    int fib[2] = { 0, 1 };
    
    sleep( 2 ); 
    printf( "fibonacchi(0) = 0\nfibonnachi(1) = 1\n" );
    for( i = 2; i < 15; ++ i )
    {
        int nextFib = fib[0] + fib[1];
        printf( "fibonacchi(%d) = %d\n", i, nextFib );
        //sleep( 2 );
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
