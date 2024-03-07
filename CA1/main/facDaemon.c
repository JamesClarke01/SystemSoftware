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

#define SECS_IN_DAY 86400

void initDaemon();
void handleSignal(int signo);
void advanceDay(struct tm* timeStruct);

int transferAllReports();
int transferReport(char* reportName);

int lockDir(const char* dirPath);
int unlockDir(const char* dirPath);

void debugLog(char* logString);
void debugLogInt(int number);

int main() {
    time_t now, transferTime;
    struct tm timeStruct;
    time(&now);
    timeStruct = *localtime(&now);

    timeStruct.tm_hour = TRANSFER_HR;
    timeStruct.tm_min = TRANSFER_MIN;
    timeStruct.tm_sec = TRANSFER_SEC;
    transferTime = mktime(&timeStruct);

    unlockDir(UPLOAD_DIR);

    initDaemon();

    while(1) {        
        sleep(1);
        time(&now);
        debugLogInt(difftime(transferTime, now));
        if(difftime(transferTime, now) == 0) {
            lockDir(UPLOAD_DIR);
            transferAllReports();
            unlockDir(UPLOAD_DIR);
            transferTime += SECS_IN_DAY; //advance transfer time by 1 day
        }
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

void debugLogInt(int number) {
    char buffer[10];

    sprintf(buffer, "%d", number);
    debugLog(buffer);
}

void handleSignal(int signo) {
    if (signo == SIGUSR1) { 
        transferAllReports();
    } else if (signo == SIGUSR2) {
        debugLog("Backing up");
    }
}

int lockDir(const char* dirPath) {

    mode_t readOnlyPermissions = 0444;

    if (chmod(dirPath, readOnlyPermissions) == 0) {
        return 0;
    } else {
        return 1;
    }
}

int unlockDir(const char* dirPath) {

    mode_t readWritePermissions = 0666;

    if (chmod(dirPath, readWritePermissions) == 0) {
        return 0;
    } else {
        return 1;
    }
}


int transferAllReports() {
    debugLog("Transfering");
    
    transferReport(DISTRIBUTION_REPORT);
    transferReport(MANUFACTURING_REPORT);
    transferReport(SALES_REPORT);
    transferReport(WAREHOUSE_REPORT);
}

int transferReport(char* reportName) {
    FILE *sourceFile, *destinationFile;
    char buffer[REPORT_SIZE];
    size_t bytesRead;
    char uploadPath[strlen(UPLOAD_DIR) + FILE_NAME_MAX_LEN];  
    char reportPath[strlen(REPORT_DIR) + FILE_NAME_MAX_LEN]; 

    strcpy(uploadPath, UPLOAD_DIR);
    strcat(uploadPath, "/");
    strcat(uploadPath, reportName);

    strcpy(reportPath, REPORT_DIR);
    strcat(reportPath, "/");
    strcat(reportPath, reportName);    
    
    //Open the source file for reading
    sourceFile = fopen(uploadPath, "rb");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return 1;
    }

    //Open the destination file for writing
    destinationFile = fopen(reportPath, "wb");
    if (destinationFile == NULL) {
        perror("Error opening destination file");
        fclose(sourceFile);
        return 1;
    }

    //Copy the contents of the source file to the destination file
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destinationFile);
    }

    //Close the files
    fclose(sourceFile);
    fclose(destinationFile);

    //Delete the file in the upload directory
    remove(uploadPath);

    return 0;
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