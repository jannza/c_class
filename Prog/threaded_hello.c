#include <stdio.h>
#include <pthread.h>

pthread_t t;
void *print_hello(void *arg);


int main( int argc, char *argv[] ){

	if(argc != 3){
		printf("Not enough arguments, I want 2\n");
		return 0;
	}

	pthread_t tid;
	pthread_create(&tid, NULL, print_hello, &argv[2]);
	pthread_join(tid, NULL);

	return 0;
	
}

void *print_hello(void *arg){
	printf("got argument %s", arg);
	printf("Threaded hello\n");
	
}
