#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "daemonConfig.h"

void createChildProcess();
void initDaemon();

int main() {
    time_t now;
    
    initDaemon();

    while(1) {        
        sleep(1);
    }
}

void initDaemon() {
    int i;
    pid_t sid;
    
    createChildProcess(); //fork to create child process

    //Create a new SID for the child process
    sid = setsid();
    if (sid < 0 ) {
        perror("Error creating new SID");
        exit(EXIT_FAILURE);
    }

    createChildProcess();  //fork a second time to ensure the process isn't a session leader

    umask(0); //Set file mode creation mask to 0

    //Change working directory to root
    if(chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    //Close all open file descriptors
    for(i = sysconf(_SC_OPEN_MAX); i >= 0; i--){
        close(i);
    }
}


void createChildProcess() {
    pid_t pid = fork();  //fork to create child process

    //Check if forking suceeded
    if (pid < 0) { 
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }

    //Exit parent process (child process ID will be 0)
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
}