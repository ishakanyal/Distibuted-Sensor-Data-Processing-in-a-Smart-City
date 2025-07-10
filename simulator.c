#include <stdio.h> //printf, snprintf
#include <stdlib.h> //exit
#include <unistd.h> //write, sleep, close
#include <fcntl.h> //open
#include <string.h> //strlen
#include <sys/stat.h> //mkfifo
#include <time.h> //real time

int main() {
    const char *fifo = "sensor_fifo"; 
    mkfifo(fifo, 0777); //fifo cret

    int fd = open(fifo, O_WRONLY); //open fifo in write mode
    if(fd == -1){
    	perror("Error in opening fifo");
    	return 1;
    }
    	
    //arr of str-sts
    const char *traffic[] ={"Traffic:HIGH", "Traffic:MEDIUM", "Traffic:LOW"};
    const char *waste[] ={"Waste:90", "Waste:60", "Waste:30"};
    const char *env[] ={"Environment:POOR", "Environment:MODERATE", "Environment:GOOD"};
    
    int i=0; //loop counter
    
    srand(time(NULL)); //random strting point, current time, time will return directly
    
    while (1){ //continous simul
        char buffer[256]; //hold final sensor data str
        int t = rand() %3;
        int w = rand() %3;
        int e = rand() %3;
        sprintf(buffer, "%s|%s|%s\n", traffic[t], waste[w], env[e]); //no overflow by limiting sizebuff
        write(fd, buffer, strlen(buffer) + 1); //write fifo
        printf("[Simulator] Sent: %s", buffer);
        i++;
        sleep(5); //5 sec delay
    }

    close(fd);
    return 0;
}
