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

#define SECS_IN_DAY 86400

//Checking for file changes

void initDaemon();
void handleSignal(int signo);
void advanceDay(struct tm* timeStruct);

int moveAllReports(char* sourceDir, char* targetDir);
int moveReport(char* sourceDir, char* targetDir, char* reportName);

int lockDir(const char* dirPath);
int unlockDir(const char* dirPath);

//Time
time_t getTransferTime();
void transferIfTime(time_t* transferTime);

int main() {
    time_t transferTime;
    
    initDaemon();

    transferTime = getTransferTime();
    int inotify_fd = setupDirMonitoring();

    while(1) {                
        sleep(1);        
        
        transferIfTime(&transferTime);

        checkDirForChanges(inotify_fd);
    }
}

void transferIfTime(time_t* transferTime) {
    time_t now;

    time(&now);
    
    if(difftime(*transferTime, now) == 0) {            
        moveAllReports(UPLOAD_DIR, REPORT_DIR);
        unlockDir(UPLOAD_DIR);
        *transferTime += SECS_IN_DAY; //advance transfer time by 1 day
    }
}

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
 

void handleSignal(int signo) {
    if (signo == SIGUSR1) {     
        moveAllReports(UPLOAD_DIR, REPORT_DIR);
        debugLog("Transferred files from upload to reporting.");
    } else if (signo == SIGUSR2) {
        moveAllReports(REPORT_DIR, BACKUP_DIR);
        debugLog("Backed up files.");
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


int moveAllReports(char* sourceDir, char* targetDir) {    
    lockDir(sourceDir);
    lockDir(targetDir);
    moveReport(sourceDir, targetDir, DISTRIBUTION_REPORT);
    moveReport(sourceDir, targetDir, MANUFACTURING_REPORT);
    moveReport(sourceDir, targetDir, SALES_REPORT);
    moveReport(sourceDir, targetDir, WAREHOUSE_REPORT);
    unlockDir(sourceDir);
    unlockDir(targetDir);
}

int moveReport(char* sourceDir, char* targetDir, char* reportName) {
    FILE *sourceFile, *destinationFile;
    char buffer[REPORT_SIZE];
    size_t bytesRead;
    char sourcePath[strlen(sourceDir) + FILE_NAME_MAX_LEN];  
    char targetPath[strlen(targetDir) + FILE_NAME_MAX_LEN]; 

    strcpy(sourcePath, sourceDir);
    strcat(sourcePath, "/");
    strcat(sourcePath, reportName);

    strcpy(targetPath, targetDir);
    strcat(targetPath, "/");
    strcat(targetPath, reportName);    
    
    //Open the source file for reading
    sourceFile = fopen(sourcePath, "rb");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return 1;
    }

    //Open the destination file for writing
    destinationFile = fopen(targetPath, "wb");
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
    
    remove(sourcePath);

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