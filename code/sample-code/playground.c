#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>		/* for setitimer */
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ
#define INTERVAL 5		/* number of milliseconds to go off */

ucontext_t cctx, nctx, main_context;
int flip = 0;
int inMain = 0;
int count = 0;
struct itimerval it_val;	/* for setting itimer */


/* function prototype */
void DoStuff(void);
void execute(void *(*function)(void*), void * arg, ucontext_t* current_context);



void*  f1withparam(){
	
	for (int i=0; i < 10000; i++){
		printf("Thread 1:%d\n", i);
	}
}

void*  f2withparam(){
	for (int i=10000; i < 20000; i++){
		printf("Thread 2:%d\n", i);
	}
}




int main(int argc, char **argv) {



	/* Upon SIGALRM, call DoStuff().
	Set interval timer.  We want frequency in ms, 
	but the setitimer call needs seconds and useconds. */
	
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


	
	
	
	if (getcontext(&cctx) < 0){
		perror("getcontext");
		exit(1);
	}

	// Allocate space for stack1	
	void *stack1=malloc(STACK_SIZE);
	
	if (stack1 == NULL){
		perror("Failed to allocate stack1");
		exit(1);
	}
      
	/* Setup context that we are going to use */
	cctx.uc_link=NULL;
	cctx.uc_stack.ss_sp=stack1;
	cctx.uc_stack.ss_size=STACK_SIZE;
	cctx.uc_stack.ss_flags=0;
	
	
	// Make the context to start running at f1withparam()
	makecontext(&cctx,(void *)&execute,3, &f1withparam, NULL, &cctx);
	





	if (getcontext(&nctx) < 0){
		perror("getcontext");
		exit(1);
	}

	// Allocate space for stack2	
	void *stack2=malloc(STACK_SIZE);
	
	if (stack2 == NULL){
		perror("Failed to allocate stack2");
		exit(1);
	}
      
	/* Setup context that we are going to use */
	nctx.uc_link=NULL;
	nctx.uc_stack.ss_sp=stack2;
	nctx.uc_stack.ss_size=STACK_SIZE;
	nctx.uc_stack.ss_flags=0;
	
	
	// Make the context to start running at f2withparam()
	makecontext(&nctx,(void *)&execute,3, &f2withparam, NULL, &nctx);

	



  	while (1) {

  		if (count == 2){
  			exit(0);
  		}
  	}

  	return 0;
}


void DoStuff(void) {

	// We are currently in a worker process
	if (inMain == 1){

		if (flip == 0){

			flip = 1;
			inMain = 0;
			swapcontext(&cctx, &main_context);
		}

		else{

			flip = 0;
			inMain = 0;
			swapcontext(&nctx, &main_context);
		}
	}

	// We are in the Main process
	else{


		if (flip == 0){

			//flip = 1;
			inMain = 1;
			swapcontext(&main_context, &cctx);
		}

		else{

			//flip = 0;
			inMain = 1;
			swapcontext(&main_context, &nctx);
		}
	}


}


void execute(void *(*function)(void*), void * arg, ucontext_t* current_context){

	inMain = 1;

	function(arg);

	count++;

	swapcontext(current_context, &main_context);
}