#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>



int main(int argc, char ** argv) {
	
	char *options[] = {"./stopper", "-s", NULL};
	execv("./stopper", options);
	exit(EXIT_SUCCESS);
}

