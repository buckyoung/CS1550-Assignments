//Custom Semaphore with Custom Sleep/Wake and Busy Waiting Version
//bcy3 -- CS1550, Spring 2014

//NOTE: Initial sizes of semaphores:
//	EMPTY[buffersize] .. FULL[0] .. MUTEX[1]
//NOTE: number_of_items is not needed in this version

//Multi-core fix

//#define THOT //My local machine does not recognize cpu_set_t, so I leave THOT undefined when compiling locally.
#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#define PROD_SIGNAL SIGUSR1 
#define CONS_SIGNAL SIGUSR2

//Structs:
typedef struct {
	int val; //value of semaphore
	int sig; //signal which will wake this up
} Semaphore;

//Prototypes:
void *start_producer(void *);
void my_sleep(int);
void my_wakeup(int);
void down(Semaphore);
void up(Semaphore);
void enter_region(pthread_t);
void leave_region(pthread_t);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index, prod_interested, cons_interested;
pthread_t prod_thread, cons_thread, last_request;
Semaphore empty, full, mutex;

int main (int argc, char* argv[]){


	int result, item;

	//Multi-core fix
	#ifdef THOT
		cpu_set_t cpuset;
	#endif

	//Custom Signals Given Code
	sigset_t set;
	int s;

	//Check Cmd-Line Args
	if(argc != 2){
		printf("Usage: ./prodcons_v# [buffer size]\n");
		return -1;
	}

	//Multi-core fix
	#ifdef THOT
     CPU_ZERO(&cpuset);
     CPU_SET(0, &cpuset);
     if(sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) != 0)
     {
           perror("Setting affinity failed\n");
           exit(-1);
     }
     #endif

    //Init Semaphores
     empty.val = buffer_size;
     full.val = 0;
     mutex.val = 1;

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
		down(full);
		down(mutex);

		//Get item
		item = shared_buffer[consumer_index];
		//Set index
		consumer_index = (consumer_index+1) % buffer_size;

		up(mutex);
		up(empty);
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
		down(empty);
		down(mutex);

		//Store it
		shared_buffer[producer_index] = seq_ints;
		//Set index
		producer_index = (producer_index+1) % buffer_size;

		up(mutex);
		up(full);
		//END-CRIT

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
void my_wakeup(int sig){	// PROD_SIGNAL or CONS_SIGNAL

	if(sig == PROD_SIGNAL) { //If trying to wake up the producer
		pthread_kill(prod_thread, PROD_SIGNAL); //send producer signal to the producer thread
	} else if(sig == CONS_SIGNAL) { //If trying to wake up the consumer
		pthread_kill(cons_thread, CONS_SIGNAL); //send the consumber signal to the consumer thread
	}
}

//Custom down
void down(Semaphore s){
	
	pthread_t thread;
	thread = pthread_self(); //get the currently running thread

	enter_region(thread);
	if(thread = prod_thread){ //if producer
		s.val -= 1;
		if(s.val < 0){
			//Save wake signal then sleep
			s.sig = PROD_SIGNAL;
			leave_region(thread);
			my_sleep(PROD_SIGNAL);
		}
	} else if (thread = cons_thread){ //if consumer
		s.val -= 1;
		if(s.val < 0){
			//save wake signal then sleep
			s.sig = CONS_SIGNAL;
			leave_region(thread);
			my_sleep(CONS_SIGNAL);
		}
	}

}

//Custom up
void up(Semaphore s){
	
	pthread_t thread;
	thread = pthread_self(); //get the currently running thread

	enter_region(thread);
	if(thread = prod_thread){ //if producer
		s.val += 1;
		if(s.val <= 0){
			//We know wake sig, so wake
			my_wakeup(PROD_SIGNAL);
		}
	} else if (thread = cons_thread){ //if consumer
		s.val += 1;
		if(s.val <= 0){
			//we know wake sig, so wake
			my_wakeup(CONS_SIGNAL);
		}
	}
	leave_region(thread);

}


//Custom enter
void enter_region(pthread_t process){
	pthread_t other;
	
	if (process == prod_thread){
		other = cons_thread;
		prod_interested = 1; //true
		last_request = process;
		while(cons_interested == 1 && last_request == process){
			//do nothing, busy wait
		}
	} else if (process == cons_thread){
		other = prod_thread;
		cons_interested = 1; //true
		last_request = process;
		while(prod_interested == 1 && last_request == process){
			//do nothing, busy wait
		}

	}
}

//Custom leave
void leave_region(pthread_t process){
	if (process == prod_thread){
		prod_interested = 0; //false
	} else if (process == cons_thread){
		cons_interested = 0; //false

	}
}





