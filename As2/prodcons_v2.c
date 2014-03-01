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
#define DEBUG 0

//Structs:
typedef struct {
	int val; //value of semaphore
	int sig; //signal which will wake this up
} Semaphore;

//Prototypes:
void *start_producer(void *);
void my_sleep(int);
void my_wakeup(int);
void down(Semaphore *);
void up(Semaphore *);
void enter_region(pthread_t);
void leave_region(pthread_t);
void debug(char *, char *, int);

//Shared variables:
int *shared_buffer;
int buffer_size, producer_index, consumer_index, prod_interested, cons_interested;
pthread_t prod_thread, cons_thread, last_request;
Semaphore *empty, *full, *mutex;

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

     //Init interested
     prod_interested = 0; //false
     cons_interested = 0; //false

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

    //Init Semaphores
    empty = malloc(sizeof(Semaphore));
    full = malloc(sizeof(Semaphore));
    mutex = malloc(sizeof(Semaphore));
     empty->val = buffer_size;
     full->val = 0;
     mutex->val = 1;

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

		debug("CONS", "Loop begin", 110);

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

		debug("PROD", "Loop begin", 144);

		//Produce an integer for the buffer
		seq_ints++;
		printf("Producer produced: %d\n", seq_ints);

		//CRIT
		debug("PROD", "empty val is INT (before down)", empty->val);
		debug("PROD", "mutex val is INT (before down)", mutex->val);
		down(empty);
		down(mutex);

		//Store it
		shared_buffer[producer_index] = seq_ints;
		//Set index
		producer_index = (producer_index+1) % buffer_size;

		debug("PROD", "mutex val is INT (after down, before up)", mutex->val);
		debug("PROD", "full val is INT (after down, before up)", full->val);
		up(mutex);
		up(full);
		debug("PROD", "mutex val is INT (after up)", mutex->val);
		debug("PROD", "full val is INT (after up)", full->val);
		//END-CRIT

	}

}

//Custom sleep given code
void my_sleep(int sig) { 	// PROD_SIGNAL or CONS_SIGNAL
    int ret; 				// ret = return value for sigwait
    sigset_t set;			// set = signalset

    debug("SLEEP", "Enter", 172);

    sigemptyset(&set);		// initialize set
    sigaddset(&set, sig); 	// add the SIGNAL to the signal set
    sigwait(&set, &ret);	// wait until the SIGNAL is sent and return

    if(ret != sig){			// sanity check 
    	printf("ERROR the return value of sigwait (%d) does not equal the signal sent (%d)", ret, sig);
    }
}

//Custom wakeup
void my_wakeup(int sig){	// PROD_SIGNAL or CONS_SIGNAL

	debug("WAKE", "Enter", 186);

	if(sig == PROD_SIGNAL) { //If trying to wake up the producer
		debug("WAKE", "wake producer", 191);
		pthread_kill(prod_thread, PROD_SIGNAL); //send producer signal to the producer thread
	} else if(sig == CONS_SIGNAL) { //If trying to wake up the consumer
		debug("WAKE", "wake consumer", 194);
		pthread_kill(cons_thread, CONS_SIGNAL); //send the consumber signal to the consumer thread
	}
}

//Custom down
void down(Semaphore *s){
	
	pthread_t thread;
	thread = pthread_self(); //get the currently running thread

	debug("DOWN", "Enter", 201);

	enter_region(thread);
	if(thread == prod_thread){ //if producer
		debug("DOWN", "semaphore value is going down (producer thread)", 207);
		s->val -= 1;
		debug("DOWN", "semaphore value is INT (check <0 to sleep producer", s->val);
		if(s->val < 0){
			//Save wake signal then sleep
			s->sig = PROD_SIGNAL;
			leave_region(thread);
			debug("DOWN", "sleep producer", 215);
			my_sleep(PROD_SIGNAL);
		}
	} else if (thread == cons_thread){ //if consumer
		debug("DOWN", "semaphore value is going down (consumer thread)", 216);
		s->val -= 1;
		debug("DOWN", "semaphore value is INT (check <0 to sleep consumer", s->val);
		if(s->val < 0){
			//save wake signal then sleep
			s->sig = CONS_SIGNAL;
			leave_region(thread);
			debug("DOWN", "sleep consumer", 224);
			my_sleep(CONS_SIGNAL);
		}
	}

}

//Custom up
void up(Semaphore *s){
	
	pthread_t thread;
	thread = pthread_self(); //get the currently running thread

	debug("UP", "Enter", 230);

	enter_region(thread);
	if(thread == prod_thread){ //if producer
		debug("UP", "semaphore value is going up (producer thread)", 236);
		s->val += 1;
		debug("UP", "semaphore value is INT (check <=0 to wake consumer)", s->val);
		if(s->val <= 0){
			//We know wake sig, so wake
			debug("UP", "wakeup consumer", 244);
			my_wakeup(CONS_SIGNAL);
		}
	} else if (thread == cons_thread){ //if consumer
		debug("UP", "semaphore value is going up (consumer thread)", 242);
		s->val += 1;
		debug("UP", "semaphore value is INT (check <=0 to wake producer)", s->val);
		if(s->val <= 0){
			//we know wake sig, so wake
			debug("UP", "wakeup producer", 252);
			my_wakeup(PROD_SIGNAL);
		}
	}
	leave_region(thread);

}


//Custom enter
void enter_region(pthread_t process){
	pthread_t other;

	debug("E_REG", "Enter", 255);
	
	if (process == prod_thread){
		debug("E_REG", "prod_interested = TRUE, last_request = producer", 260);
		other = cons_thread;
		prod_interested = 1; //true
		last_request = process;
		if(cons_interested == 1 && last_request == process) //DEBUG
			debug("E_REG", "about to busy wait the producer", 279);
		
		while(cons_interested == 1 && last_request == process){
			//do nothing, busy wait
		}
	} else if (process == cons_thread){
		debug("E_REG", "cons_interested = TRUE, last_request = consumer", 268);
		other = prod_thread;
		cons_interested = 1; //true
		last_request = process;

		if(prod_interested == 1 && last_request == process) //DEBUG
			debug("E_REG", "about to busy wait the consumer", 279);

		while(prod_interested == 1 && last_request == process){
			//do nothing, busy wait
		}


	}
}

//Custom leave
void leave_region(pthread_t process){

	debug("L_REG", "Enter", 279);

	if (process == prod_thread){
		debug("L_REG", "prod_interested = FALSE", 286);
		prod_interested = 0; //false
	} else if (process == cons_thread){
		debug("L_REG", "cons_interested = FALSE", 289);
		cons_interested = 0; //false

	}
}

void debug(char * tag, char * msg, int line){

	if (DEBUG)
		printf("(debug %d) [%lu] %s: %s\n", line, (unsigned long)pthread_self(), tag, msg);

}





