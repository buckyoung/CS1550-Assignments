#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Prototypes:
void *start_producer(void *);

//Globals:
int shared_index;
int *shared_buffer;

int main (int argc, char* argv[]){

	pthread_t prod_thread;
	int buffer_size, result;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Convert Cmd-Line Arg into an integer and malloc space for the buffer based on cmd-line size
	buffer_size = atoi(argv[1]);
	shared_buffer = malloc(sizeof(int) * buffer_size);

	//Consumer used main thread, Produces uses new thread
	result = pthread_create(&prod_thread, NULL, start_producer, (void *)&buffer_size);

	//Check if error on pthread create
	if (result != 0){
		printf("Error: Cannot create producer thread. Exiting...\n");
		return result;
	}

	//Initialize the shared index variable
	shared_index = 0;

	printf("Hello from consumer.\n"); //DEBUG

	//Infinite CONSUMER loop
	while(1){ 

		if(shared_index > 0){
			printf("------ Consumer consumed: %d\n", shared_buffer[--shared_index]);
		}

	}

	pthread_yield();

	return 0;


}

void *start_producer(void *args){

	int seq_ints, buffer_size; //sequential integers
	seq_ints=0;

	buffer_size = *(int*)args;

	printf("Hello from producer.\n"); //DEBUG

	//Infinite PRODUCER loop
	while(1){ 

		if( shared_index < buffer_size ){ //Check that the index is less than the buffer size, else dont produce
			shared_buffer[shared_index++] = seq_ints;
			printf("Producer produced: %d\n", seq_ints);
			seq_ints++;
		}

	}

}