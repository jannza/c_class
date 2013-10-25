#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int c_count = 0;
sigset_t     newmask, oldmask;
sigemptyset(&newmask);

void signal_handler(int sigint){
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0){
			printf("SIG_BLOCK error");
		}
	if(sigint == 2){
		c_count++;
		//printf("caught ctrl-c\n");
		printf("ctrl-c presses %d\n", c_count);
	}
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0){
			 printf("SIG_SETMASK error");
		}
}

main(){
	
	signal(SIGINT, signal_handler);
	while(1){
		if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0){
			printf("SIG_BLOCK error");
		}		


		printf(".\n");

		if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0){
			 printf("SIG_SETMASK error");
		}

		usleep(500000);

	}
	return 0;
}

