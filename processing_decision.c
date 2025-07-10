#include <stdio.h> //i/o
#include <stdlib.h> //memo func: malloc, free, strdup
#include <pthread.h> //threads and syn
#include <string.h> //strstr, strdup
#include <sys/ipc.h> //ipc
#include <sys/msg.h> //msg que
#include <sys/types.h>

#define MAX 100 //const val 

//msg format for que
struct msg_buffer{
    long msg_type; 
    char msg_text[MAX]; //sensor data
};

//prevents mixing of thread when multiple threads print o/p 
pthread_mutex_t lock; //declare

//thread fun 
void* process_data(void* arg){ //returns void*, null-nothing
    struct msg_buffer* msg = (struct msg_buffer*) arg; //casting for accessing fields

    pthread_mutex_lock(&lock); //mutex lock, prevent
    
   //decision module
   //traffic
   if(msg->msg_type == 1){
   	    printf("[Traffic] %s\n", msg->msg_text);
	    if (strstr(msg->msg_text, "HIGH")){ //find substr inside str
		printf("[Decision]: Alert, Divert traffic!!\n");
	    }else if(strstr(msg->msg_text, "MEDIUM")){
		printf("[Decision]: Traffic Flow Medium!!\n");
	    }else if(strstr(msg->msg_text, "LOW")){
		printf("[Decision]: Traffic Flow Normal!!\n");
	    }
     }

    //waste
    else if(msg->msg_type == 2){
             printf("[Waste] %s\n", msg->msg_text);
	    if (strstr(msg->msg_text, "90")){
		printf("[Decision]: ALert, Dispatch Waste Pickup!!\n");
	    }else if (strstr(msg->msg_text, "60")){
		printf("[Decision]: Dispatch Waste Pickup!!\n");
	    }else if(strstr(msg->msg_text, "30")){
		printf("[Decision]: Skip Pickup!!\n");
	    }
    }

    //environment
    else if(msg->msg_type == 3){
    	    printf("[Environment] %s\n", msg->msg_text);
	    if (strstr(msg->msg_text, "POOR")) {
		printf("[Decision]: Alert Pollution Control!!\n");
	    }else if (strstr(msg->msg_text, "MODERATE")) {
		printf("[Decision]: Action Needed!!\n");
	    }else if (strstr(msg->msg_text, "GOOD")){
		printf("[Decision]: No Action Needed!!\n");
	    }
     }
     
    pthread_mutex_unlock(&lock); //unlock, thread end
    
    free(msg); //free dy memo
    return NULL;
}

int main() {
    
    int msgid = msgget(9690, IPC_CREAT | 0777 );
   
    pthread_mutex_init(&lock, NULL); //inti mutex

    //infinite loop
    while(1){ 
   	
  	for(int i = 1; i <=3; i++){//3 times for msg type
  		//allocation memo to hold msg str, pass data to the threads 
  		struct msg_buffer *message = malloc(sizeof(struct msg_buffer)); 
       		if(!message) continue; //fail, skip
       		
       		//return = valid size, return -1 for error	
		ssize_t result = msgrcv(msgid, message, sizeof(message->msg_text), i, 0); //rcv msg of any type from que
		if(result >0){ //+ve
			message->msg_type = i;
			pthread_t t; //new thread
			//ne wthread which run fun pro_data, give msg str
			pthread_create(&t, NULL, process_data, message); 
			pthread_detach(t); //clean thread resources automatically after it finish, work independently
		} else{
			free(message); //if msg not received then we'll free it to prevent memo leakage
		}
	}
    }

    pthread_mutex_destroy(&lock); //destroy mutex 
    return 0;
}
