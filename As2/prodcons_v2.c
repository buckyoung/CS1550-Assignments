#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Prototypes:
void *start_producer(void *);
void my_sleep(int);
void my_wakeup(int);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index, number_of_items;

int main (int argc, char* argv[]){

	pthread_t prod_thread, cons_thread;
	int result, item;

	//Custom Signals Given Code
	sigset_t set;
	int s;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Init consumer thread id
	cons_thread = pthread_self();
	//Custom Signal Given Code
	sigemptyset(&set);
	sigaddset(&set, PROD_SIGNAL);
	sigaddset(&set, CONS_SIGNAL);
	s = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (s != 0){
		printf("Error during signal creation [pthread_sigmask]. Exiting...\n");
		return s;
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

		if(number_of_items != 0){
			item = shared_buffer[consumer_index];
			consumer_index = (consumer_index+1) % buffer_size;
			number_of_items--;

			//Consume it (print it out)
			printf("- - - - Consumer consumed: %d\n", item);

		}

	}

	return 0;


}

void *start_producer(void *args){

	int seq_ints; //sequential integers
	seq_ints=-1; //starts the production at 0!

	//Infinite PRODUCER loop
	while(1){ 

		if(number_of_items != buffer_size){ //Rudamentary
			//Produce an integer for the buffer
			seq_ints++;
			printf("Producer produced: %d\n", seq_ints);

			//Store it
			shared_buffer[producer_index] = seq_ints;

			//Set index
			producer_index = (producer_index+1) % buffer_size;

			//Increment item count
			number_of_items++;
		}

	}

}

//Custom sleep given code
void my_sleep(int who) {
    int sig;
    sigemptyset(&set);
    sigaddset(&set, who);
    sigwait(&set, &sig);
}

//Custom wakeup
void my_wakeup(int who){
	pthread_kill(pthread_t thread, int signal);
}








