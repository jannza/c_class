#include <stdio.h>
#include <pthread.h>

pthread_t t;
void *counter(void *arg);
int cnt = 0;
int iterations =1000000;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(){
	pthread_t tid1;
	pthread_t tid2;
	pthread_create(&tid1, NULL, counter, NULL);
	pthread_create(&tid2, NULL, counter, NULL);

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	printf("%d counts\n", cnt);
	pthread_detach(tid1);
	pthread_detach(tid2);
	pthread_exit(NULL); 
	return 0;
	
}

void *counter(void *arg){

	int i;
	for(i=0;i<iterations;i++){
		pthread_mutex_lock(&mutex);
	
		cnt++;

		pthread_mutex_unlock(&mutex);
	}

}
