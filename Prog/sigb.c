#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int c_count = 0;

void signal_handler(int sigint){
	
	if(sigint == 2){
		c_count++;
		//printf("caught ctrl-c\n");
		printf("ctrl-c presses %d\n", c_count);
	}
	
}

main(){
	

	signal(SIGINT, signal_handler);
	while(1){
		
		printf(".\n");
		usleep(500000);

	}
	return 0;
}

