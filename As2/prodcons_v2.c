//Custom Semaphore with Custom Sleep/Wake and Busy Waiting Version
//bcy3 -- CS1550, Spring 2014

//NOTE: Initial sizes of semaphores:
//	EMPTY[buffersize] .. FULL[0] .. MUTEX[1]
//NOTE: number_of_items is not needed in this version

//Multi-core fix (2 lines)
#define _GNU_SOURCE
#define <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#define PROD_SIGNAL SIGUSR1 //?? THOT DOES NOT RECOGNIZE THESE ??
#define CONS_SIGNAL SIGUSR2

#define THOT 1


//Prototypes:
void *start_producer(void *);
void my_sleep(int);
void my_wakeup(int);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index;

int main (int argc, char* argv[]){

	pthread_t prod_thread, cons_thread;
	int result, item;
	if(THOT){
		cpu_set_t cpuset;//Multi-core fix
	}

	//Custom Signals Given Code
	sigset_t set;
	int s;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Multi-core fix (2 lines)
	if(THOT){
     	CPU_ZERO(&cpuset);
     	CPU_SET(0, &cpuset);
 	}

     if(sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) != 0)
     {
           perror("Setting affinity failed\n");
           exit(-1);
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
	producer_index = consumer_index = 0;

	//Infinite CONSUMER loop
	while(1){ 


		//CRIT
		FULL.down(); //Psuedo-code
		MUTEX.down(); //Psuedo-code

		//Get item
		item = shared_buffer[consumer_index];
		//Set index
		consumer_index = (consumer_index+1) % buffer_size;

		MUTEX.up(); //Psuedo-code
		EMPTY.up(); //Psuedo-code
		//END-CRIT


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

		//CRIT
		EMPTY.down(); //Psuedo-code
		MUTEX.down(); //Psuedo-code

		//Store it
		shared_buffer[producer_index] = seq_ints;
		//Set index
		producer_index = (producer_index+1) % buffer_size;

		MUTEX.up(); //Psuedo-code
		FULL.up(); //Psuedo-code
		//END-CRIT


	}

}

//Custom sleep given code
void my_sleep(int who) {
    int sig;
    sigemptyset(&set); //?? SET ??
    sigaddset(&set, who); //?? WHO ??  
    sigwait(&set, &sig); //?? SIG ?? 
}

//Custom wakeup
void my_wakeup(int who){
	pthread_kill(pthread_t thread, int signal); //?? What signal ??
}








