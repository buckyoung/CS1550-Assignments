//Unsync Version
//bcy3 -- CS1550, Spring 2014

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

//#define TEST //DEFINE IF TESTING TIME (limited loops)
#define LOOP_NUM 500000 //five-hundred thousand

#define PROD_SIGNAL SIGUSR1 
#define CONS_SIGNAL SIGUSR2

//Prototypes:
void *start_producer(void *);
void my_sleep(int);
void my_wakeup(pthread_t, int);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index, number_of_items;
pthread_t prod_thread, cons_thread;


int main (int argc, char* argv[]){

	sigset_t set;
	int s;

	int result, item;

	int loop_num;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Init loop num
	loop_num = LOOP_NUM;

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
	while(loop_num){ 

		//Check if there is nothing to consume
		if(number_of_items == 0){
			my_sleep(CONS_SIGNAL);
		}

		//given code
		item = shared_buffer[consumer_index];
		consumer_index = (consumer_index+1) % buffer_size;
		number_of_items--;

		#ifdef TEST
			loop_num--;
		#endif

		//check if we should wake up the producer
		if(number_of_items == buffer_size-1){
			my_wakeup(prod_thread, PROD_SIGNAL);
		}

		//Consume it (print it out)
		printf("- - - - Consumer consumed: %d\n", item);

	}

	return 0;


}

void *start_producer(void *args){

	int seq_ints; //sequential integers
	seq_ints=-1; //starts the production at 0!

	//Infinite PRODUCER loop
	while(1){ 

		//Produce an integer for the buffer
		seq_ints++;
		printf("Producer produced: %d\n", seq_ints);

		//Check if buffer is full
		if (number_of_items == buffer_size){
			my_sleep(PROD_SIGNAL);
		}

		//Store it
		shared_buffer[producer_index] = seq_ints;

		//Set index
		producer_index = (producer_index+1) % buffer_size;

		//Increment item count
		number_of_items++;

		//Check if we have an item for the consumer after it's been empty
		if (number_of_items == 1){
			my_wakeup(cons_thread, CONS_SIGNAL);
		}
		
	}

}

//Custom sleep given code
void my_sleep(int sig) { 	// PROD_SIGNAL or CONS_SIGNAL
    int ret; 				// ret = return value for sigwait
    sigset_t set;			// set = signalset

    sigemptyset(&set);		// initialize set
    sigaddset(&set, sig); 	// add the SIGNAL to the signal set
    sigwait(&set, &ret);	// wait until the SIGNAL is sent and return

    if(ret != sig){			// sanity check 
    	printf("ERROR the return value of sigwait (%d) does not equal the signal sent (%d)", ret, sig);
    }
}

//Custom wakeup
void my_wakeup(pthread_t thread, int sig){	// thread= prod_thread or cons_thread // sig = PROD_SIGNAL or CONS_SIGNAL
		pthread_kill(thread, sig); //send producer signal to the producer thread
}



