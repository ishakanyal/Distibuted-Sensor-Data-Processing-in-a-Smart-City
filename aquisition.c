#include <stdio.h> //printf, perror
#include <stdlib.h> //exit
#include <string.h> //strncpy
#include <unistd.h> //read, close, mkfifo
#include <fcntl.h> //file cntrls: O_RDONLY
#include <sys/msg.h> //msg que
#include <sys/ipc.h> //ipc
#include <sys/types.h> //system data types: key_t
#include <sys/stat.h> //mkfifo

#define MAX 100

//struct for ipc
struct msg_buffer{
    long msg_type; 
    char msg_text[MAX];
};

int main() {
    const char *fifo = "sensor_fifo"; 
    mkfifo(fifo, 0777);

    int fd = open(fifo, O_RDONLY); //opn fifo in read mode
     if(fd == -1){
    	perror("Error in opening fifo");
    	return 1;
    }
    	
    int msgid = msgget(9690, IPC_CREAT | 0777); //key assign
    if(msgid == -1){
    	perror("Error in creating/accessing msg que");
    	return 1;
    }
    
    char buffer[MAX]; //temp, reading fifo
    struct msg_buffer message;

    while(1){
        read(fd, buffer, sizeof(buffer)); //read data frm fifo into buff
        
        //sensor string divide
        char traffic[MAX], waste[MAX], env[MAX];
        //reads data from string- char arr
        sscanf(buffer, "%[^|]|%[^|]|%[^\n]", traffic, waste, env); //traffic| waste| env
        
        //desg, source, maxlen
        message.msg_type = 1;     
        strncpy(message.msg_text, traffic, MAX); //cpoy into msg struct
        msgsnd(msgid, &message, sizeof(message.msg_text), 0); //send msg to que

	message.msg_type = 2;     
        strncpy(message.msg_text, waste, MAX); //cpoy into msg struct
        msgsnd(msgid, &message, sizeof(message.msg_text), 0); //send msg to que
        
        message.msg_type = 3;     
        strncpy(message.msg_text, env, MAX); //cpoy into msg struct
        msgsnd(msgid, &message, sizeof(message.msg_text), 0); //send msg to que
        
        printf("[Acquisition] Forwarded: [%s] [%s] [%s]\n", traffic, waste, env);
    }

    close(fd); //close file des
    return 0;
}
