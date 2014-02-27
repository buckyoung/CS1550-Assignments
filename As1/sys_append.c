
#define USERLEN 20
#define MSGLEN 140

struct my_node{
	char to[USERLEN];
	char from[USERLEN];
	char msg[MSGLEN];
	struct my_node *prev;
	struct my_node *next;
};

struct my_node *head;

//Return 0 success //Return negative for failure
asmlinkage long sys_cs1550_send_msg(const char __user *to, const char __user *msg, const char __user *from)
{
	struct my_node *tmp;
	struct my_node *new;

	//Create new node
	new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
	if (new == NULL){ //kmalloc failed
		return -1; //fail
	}
	//Init node
	strncpy(new->to, to, USERLEN); //copy TO name
	strncpy(new->from, from, USERLEN); //copy FROM name
	strncpy(new->msg, msg, MSGLEN); //copy MSG
	new->prev = NULL;
	new->next = NULL;
	
	if (head == NULL){ //Set head if it is NULL
		head = new;
	} else { //Else head points to a linked list, so traverse and find end
		tmp = head; //start at head
		while(tmp->next != NULL){ //search for end
			tmp = tmp->next;
		}
		//Now tmp contains the last node
		tmp->next = new;
		new->prev = tmp;
	}

   	return 0; //success
}

//Return 1 to call again //Return 0 to not call again //Return negative for failure
asmlinkage long sys_cs1550_get_msg(const char __user *to, char __user *msg, size_t msg_len, char __user *from, size_t from_len)
{
	struct my_node *tmp;
	int result;
	result = 0;
	//Cycle through linked list and find all msg's for user TO
	//When one is found, continue looking for another to determine return value

	//Start at head
	if(head == NULL){
		return 0; //No messages at all
	} else {
		tmp = head;
	}

	do{
		if(!strcmp(to, tmp->to)){ //if they are equal
			//tmp msg belongs to this user! send it and delete it
			//send it:
			strncpy(msg, tmp->msg, msg_len);
			strncpy(from, tmp->from, from_len);
			//delete it:
			//Four cases:
			if(tmp->next == NULL && tmp->prev == NULL){ //Single-element head
				head = NULL;
			} else if (tmp->next != NULL && tmp->prev != NULL){ //Middle of linked list
				(tmp->next)->prev = tmp->prev;
				(tmp->prev)->next = tmp->next;
			} else if (tmp->next == NULL && tmp->prev != NULL){ //End of linked list
				(tmp->prev)->next = NULL;
			} else if (tmp->next != NULL && tmp->prev == NULL){ //Beginning of linked list
				head = tmp->next;
				(tmp->next)->prev = NULL;
			}
			kfree(tmp);
			break;
		}
		//Iterate
		tmp = tmp->next;
	} while (tmp != NULL);


	//The above will return a single message
	//We now must determine the return value, 1 if more messages waiting
	
	//Start at head 
	if(head == NULL){
		return 0; //No more messages
	} else {
		tmp = head;
	}

	do{
		if(!strcmp(to, tmp->to)){ //if they are equal
			result = 1;
			break;
		}
		//Iterate
		tmp = tmp->next;
	} while (tmp != NULL);

	return result; //0 or 1
}