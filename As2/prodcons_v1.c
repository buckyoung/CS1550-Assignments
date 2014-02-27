#include <stdio.h>
#include <pthread.h>

//Prototypes:
void *start_producer(void *);

//Globals:
int shared_index;
int *shared_buffer;

int main (int argc, char* argv[]){

	pthread_t prod_thread;
	int buffer-size;

	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer-size]\nExiting...\n");
		return -1;
	}

	buffer-size = atoi(argv[1]);

	shared_index = 0;
	shared_buffer = malloc(sizeof(int) * buffer-size);

	//Consumer used main thread, Produces uses new thread
	pthread_create(&prod_thread, NULL, start_producer, NULL);

	printf("Hello from consumer.\n");


}

void *start_producer(void *args){

	printf("Hello from producer.\n");

}