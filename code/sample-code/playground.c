#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>		/* for setitimer */
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ
#define INTERVAL 500		/* number of milliseconds to go off */

/* function prototype */
void DoStuff(void);

void*  f1withparam(ucontext_t *nctx){
	puts("Donald- you are threaded\n");
	setcontext(nctx);
	/* setcontext sets PC <--value in nctx context*/
}

ucontext_t cctx,nctx;

int main(int argc, char **argv) {

	struct itimerval it_val;	/* for setting itimer */

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


	
	
	if (argc != 1) {
		printf(": USAGE Program Name and no Arguments expected\n");
		exit(1);
	}
	
	if (getcontext(&cctx) < 0){
		perror("getcontext");
		exit(1);
	}

	// Allocate space for stack	
	void *stack=malloc(STACK_SIZE);
	
	if (stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}
      
	/* Setup context that we are going to use */
	cctx.uc_link=NULL;
	cctx.uc_stack.ss_sp=stack;
	cctx.uc_stack.ss_size=STACK_SIZE;
	cctx.uc_stack.ss_flags=0;
	
	puts("allocate stack, attach func, with 1 parameter");
	
	// Make the context to start running at f1withparam()
	makecontext(&cctx,(void *)&f1withparam,1,&nctx);
	
	puts("Successfully modified context");
	
	/* swap context will activate cctx and store location after swapcontext in nctx */
	swapcontext(&nctx,&cctx);

	/* PC value in nctx will point to here */
	puts("swap  context executed correctly \n");




  	while (1) {
  		
  	}

  	return 0;
}


void DoStuff(void) {

	swapcontext(&nctx,&cctx);

}
