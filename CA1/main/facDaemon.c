#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "daemonConfig.h"
#include <fcntl.h>
#include <string.h>

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
    int fd = open("//home/SystemSoftware/CA1/main/debugLog.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    time_t currentTime = time(NULL);

    struct tm *timeStruct = localtime(&currentTime);

    char timeBuffer[22];  //yyyy-mm-dd hh:mm:ss\0
    char logBuffer[100];
    
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", timeStruct);

    sprintf(logBuffer, "[%s] %s\n", timeBuffer, logString);

    
    write(fd, logBuffer, 23 + strlen(logString));
    close(fd);
}

void handleSignal(int signo) {
    if (signo == SIGUSR1) { 
        debugLog("Transfering");
    } else if (signo == SIGUSR2) {
        debugLog("Backing up");
    }
}

void initDaemon() {
    int i;
    pid_t pid, sid;
    FILE *fp;
    
    debugLog("Starting Daemon");

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

        //save PID to file
        fp = fopen("facDaemon.pid", "w");
        if(fp != NULL) {
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
    signal(SIGUSR1, handleSignal); //for transfer
    signal(SIGUSR2, handleSignal); //for backup

    debugLog("Daemon Started");
}