#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#include "daemonConfig.h"
#include "logger.h"
#include "fileTransfer.h"
#include "signal.h"

time_t getTransferTime() {
    time_t now, transferTime;
    struct tm timeStruct;
    time(&now);
    timeStruct = *localtime(&now);

    timeStruct.tm_hour = TRANSFER_HR;
    timeStruct.tm_min = TRANSFER_MIN;
    timeStruct.tm_sec = TRANSFER_SEC;
    transferTime = mktime(&timeStruct);    

    return transferTime;    
}

time_t getUploadTime() {
    time_t now, transferTime;
    struct tm timeStruct;
    time(&now);
    timeStruct = *localtime(&now);

    timeStruct.tm_hour = UPLOAD_HR;
    timeStruct.tm_min = UPLOAD_MIN;
    timeStruct.tm_sec = UPLOAD_SEC;
    transferTime = mktime(&timeStruct);    

    return transferTime;    
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