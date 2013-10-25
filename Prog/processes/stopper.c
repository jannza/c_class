#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define TIMER 5

int main(int argc, char ** argv) {
	int time = TIMER;
	if(argc != 2 || argv[1][0] != '-' || argv[1][1] != 's') {
		printf("Usage: stopper -s\n");
		exit(EXIT_FAILURE);
	}
	while(time) {
		printf("stopper: Stopping in %d seconds.\n", time--);
		sleep(1);
	}
	puts("stopper: Stopped.");
	exit(EXIT_SUCCESS);
}

