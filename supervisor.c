#include <stdio.h> //printf, perror
#include <stdlib.h> //exit, malloc
#include <unistd.h> //fork, execl, sleep
#include <signal.h> //signal, sigchld
#include <sys/wait.h> //wait
#include <sys/msg.h> //msgget, msgctl
#include <string.h> //strcpy

#define MAX_PROCESSING 3 //max processing modules that can run in parallel
#define LOAD_THRESHOLD 6 //max no. of messages allowed in the que

pid_t processing_pids[MAX_PROCESSING]; //stores pid of running child
int num_processings = 0; //track currently running pro

int msgid; //id for interacting with que

//checkes how many msg are waiting in the que
int get_queue_length(){ 
    struct msqid_ds buf;
    if (msgctl(msgid, IPC_STAT, &buf) == -1){ //contrl msg ques
        perror("msgctl");
        return -1;
    }
    return buf.msg_qnum; //number of unread msgs by msgctl
}

//new peocessing mod start
void start_processing(){
    if (num_processings >= MAX_PROCESSING) return; //if already max3, don't create

    pid_t pid = fork(); //child process create
    //replacing child with processing exe
    if (pid == 0) {
        execl("./processing_decision", "processing_decision", NULL);
        perror("execl failed");
        exit(1);
    } 
    //stores child pid
    else if (pid > 0){
        printf("[Supervisor] Started processing PID: %d\n", pid);
        processing_pids[num_processings++] = pid; //stores child pid in the processing_pid arr and increment
    }
}

//child dies, sigchld received
void sigchld_handler(int signo){
    int status;
    pid_t pid = wait(&status); //wait gets pid and status of terminated child

    //remove dead pid from list
    for(int i = 0; i < num_processings; ++i){ //find dead pid
        if (processing_pids[i] == pid){
            printf("[Supervisor] Processing PID %d died. Restarting..\n", pid);
            //by shifting array remove dead child
            for(int j = i; j < num_processings - 1; ++j)
                processing_pids[j] = processing_pids[j + 1];
            num_processings--; //decr count
            start_processing(); //strat new proc to replace dead chlid
            break;
        }
    }
}

int main(){
    signal(SIGCHLD, sigchld_handler); //register signal to handle dead child

    msgid = msgget(9690,IPC_CREAT | 0777);
    if (msgid == -1){
        perror("msgget");
        return 1;
    }

    //start one processing
    start_processing();

    //runs infinite
    while(1){
        sleep(5); //check every 5 seconds

        int load = get_queue_length(); //get and prints current msg que load
        printf("[Supervisor] Queue load: %d messages\n",load); 

        if (load > LOAD_THRESHOLD && num_processings < MAX_PROCESSING){ //if too high load and max != 3
            printf("[Supervisor] High load detected. Creating additional processing.\n"); 
            start_processing(); //create new processing child
        }
    }

    return 0;
}
