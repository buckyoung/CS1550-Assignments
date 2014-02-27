#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <asm/unistd.h>

#define MSG_LEN 140
#define FROM_LEN 20

int main (int argc, char* argv[]){
	int result = 0;
	char msg_buffer[MSG_LEN];
	char from_buffer[FROM_LEN];
	
	msg_buffer[0] = '\0';

	if( argc == 2 && !strcmp(argv[1], "-r") ) { //RECEIVE! (get)
		//Get syscall
		//Will return 1 to call again or 0 to not
		do{
			result = syscall(__NR_cs1550_get_msg, getenv("USER"), msg_buffer, MSG_LEN, from_buffer, FROM_LEN);
			if (msg_buffer[0] != '\0'){ //if the message has been set -- this protects against no messages returned for user
				printf("Message from %s: %s\n", from_buffer, msg_buffer);
				msg_buffer[0] = '\0';
			}
		} while(result == 1);

		if(result < 0){ //error occurred
			printf("[system call failure to receive message]\n");
			return -1; //fail
		}

		printf("[all messages received]\n");

	} else if ( argc == 4 && !strcmp(argv[1], "-s")) { //SEND!
		//Send syscall
		result = syscall(__NR_cs1550_send_msg, argv[2], argv[3], getenv("USER"));
		
		if (result == 0) { //success
			printf("[message sent]\n");
		} else if(result < 0) { //failure
			printf("[system call failure to send message]\n");
			return -1; //fail
		}




	} else { //cmd-line arg's not recognized
		printf("Receive Usage: %s -r\n", argv[0]);
		printf("   Send Usage: %s -s to_user \"msg\"\n", argv[0]);
		printf("   Note: This system supports a msg length of %d characters. Your message may be truncated.\n", MSG_LEN);
		return 1; //Fail
	}

	return 0;
}