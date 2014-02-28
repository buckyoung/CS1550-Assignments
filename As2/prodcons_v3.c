//Pthread Mutex and Condition Version
//bcy3 -- CS1550, Spring 2014

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Prototypes:
void *start_producer(void *);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index, number_of_items;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prod_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cons_cond = PTHREAD_COND_INITIALIZER;

int main (int argc, char* argv[]){

	pthread_t prod_thread;
	int result, item;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Convert Cmd-Line Arg into an integer and malloc space for the buffer based on cmd-line size
	buffer_size = atoi(argv[1]);
	shared_buffer = malloc(sizeof(int) * buffer_size);

	//Consumer used main thread, Produces uses new thread
	result = pthread_create(&prod_thread, NULL, start_producer, NULL);

	//Check if error on pthread create
	if (result != 0){
		printf("Error: Cannot create producer thread. Exiting...\n");
		return result;
	}

	//Initialize the shared variables
	producer_index = consumer_index = number_of_items = 0;

	//Infinite CONSUMER loop
	while(1){ 

		pthread_mutex_lock(&mutex);

		if(number_of_items == 0){ //if no more items left
			pthread_cond_wait(&cons_cond, &mutex); //sleep this
		}

		item = shared_buffer[consumer_index];
		consumer_index = (consumer_index+1) % buffer_size;
		number_of_items--;

		//Consume it (print it out)
		printf("- - - - Consumer consumed: %d\n", item);

		if(number_of_items == buffer_size-1){ // if there is room in the buffer
			pthread_cond_signal(&prod_cond); //wakeup producer
		}

		pthread_mutex_unlock(&mutex);

	}

	return 0;


}

void *start_producer(void *args){

	int seq_ints; //sequential integers
	seq_ints=-1; //starts the production at 0!

	//Infinite PRODUCER loop
	while(1){ 

		pthread_mutex_lock(&mutex);

		if(number_of_items == buffer_size){ //if buffer is full
			pthread_cond_wait(&prod_cond, &mutex); //sleep this
		}

		//Produce an integer for the buffer
		seq_ints++;
		printf("Producer produced: %d\n", seq_ints);

		//Store it
		shared_buffer[producer_index] = seq_ints;

		//Set index
		producer_index = (producer_index+1) % buffer_size;

		//Increment item count
		number_of_items++;

		if(number_of_items == 1){ //if we have an item to consume
			pthread_cond_signal(&cons_cond); //wakeup consumer
		}

		pthread_mutex_unlock(&mutex);
		

	}

}