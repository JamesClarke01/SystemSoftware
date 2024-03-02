#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "daemonConfig.h"

void initDaemon();
void handleSignal(int signo);
void transfer();
void debugLog(char* logString);

int main() {
    time_t now;
    
    initDaemon();

    while(1) {        
        sleep(1);
    }
}

void debugLog(char* logString) {
    FILE *fp = fopen("debugLog.txt", "a");
    time_t currentTime = time(NULL);

    struct tm *timeStruct = localtime(&currentTime);

    char timeBuffer[20];  //yyyy-mm-dd hh:mm:ss\0
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", timeStruct);

    fprintf(fp, "[%s] %s\n", timeBuffer, logString);
}

void handleSignal(int signo) {
    if(signo == SIGUSR1) {
        debugLog("Beginning transfer process...\n");
    }
}


void initDaemon() {
    int i;
    pid_t pid, sid;
    FILE *fp;

    //fork to create child process
    pid = fork();  //fork to create child process

    //Check if forking suceeded
    if (pid < 0) { 
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }

    //Exit parent process (child process ID will be 0)
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    //Create a new SID for the child process
    sid = setsid();
    if (sid < 0 ) {
        perror("Error creating new SID");
        exit(EXIT_FAILURE);
    }

    //Fork a second time to ensure the process isn't a session leader
    pid = fork();

    //Check if forking suceeded
    if (pid < 0) { 
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }

    //Exit parent process (child process ID will be 0)
    if (pid > 0) {
        debugLog("Saving to file");
        //save PID to file
        fp = fopen("facDaemon.pid", "w");
        if(fp != NULL) {
            debugLog("File open succeeded");
            fprintf(fp, "%d", pid);
            fclose(fp);
        } 

        exit(EXIT_SUCCESS);
    }


    umask(0); //Set file mode creation mask to 0

    //Change working directory to root
    if(chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    //Close all open file descriptors
    for(i = sysconf(_SC_OPEN_MAX); i >= 0; i--){
        close(i);
    }
    
    // Set up signal handling
    signal(SIGUSR1, handleSignal);
}