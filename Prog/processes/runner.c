#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>



int main(int argc, char ** argv) {
	
	//create arguments
	
	char *options[] = {"./stopper", "-s", NULL};



	pid_t pid=fork();
	if( pid == 0){
		execv("./stopper", options);

	}else{
		waitpid(pid, NULL, 0);
		printf("runner:Stopped!\n");
	}



	exit(EXIT_SUCCESS);
}

