#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/inotify.h>
#include <pwd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include "daemonConfig.h"
#define EVENT_BUFFER_SIZE (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void debugLog(char* logString) {
    FILE* logFile;

    logFile = fopen(DEBUG_LOG_PATH, "a");

    time_t currentTime = time(NULL);

    struct tm *timeStruct = localtime(&currentTime);

    char timeBuffer[22];  //yyyy-mm-dd hh:mm:ss\0
    char logBuffer[100];
    
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", timeStruct);

    sprintf(logBuffer, "[%s] %s\n", timeBuffer, logString);
    
    fwrite(logBuffer, 1, 23 + strlen(logString), logFile);

    fclose(logFile);    
}

void debugLogInt(int number) {
    char buffer[10];

    sprintf(buffer, "%d", number);
    debugLog(buffer);
}

void logChanges(struct inotify_event *event) {
    FILE* logFile;
    struct stat fileInfo;
    char directoryPath[strlen(UPLOAD_DIR) + FILE_NAME_MAX_LEN];    
    time_t currentTime = time(NULL);
    struct tm *timeStruct = localtime(&currentTime);
    char user[20];

    char logBuffer[100];
    char timeBuffer[22];  //yyyy-mm-dd hh:mm:ss\0

    char fileStatus[9]; //Created/Modified

    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", timeStruct);

    //Get user name
    strcpy(directoryPath, UPLOAD_DIR);
    strcat(directoryPath, "/");
    strcat(directoryPath, event->name);
    
    if(stat(directoryPath, &fileInfo) == 0) {  //Load fileinfo 
        struct passwd *pwd = getpwuid(fileInfo.st_uid);
        
        if(pwd != NULL) {          
            strcpy(user, pwd->pw_name);                             
        }
    } 

    //Get created/modified
    if (event->mask & (IN_CREATE)) {
        strcpy(fileStatus, "Created");
    } else if (event->mask & (IN_MODIFY)) {
        strcpy(fileStatus, "Modified");
    }

    sprintf(logBuffer, "[%s] Filename: %s \tStatus: %s \tUser: %s\n", timeBuffer, event->name, fileStatus, user);
    logFile = fopen(FILE_LOG_PATH, "a");
    fwrite(logBuffer, 1,strlen(logBuffer), logFile);
    fclose(logFile);
}

int setupDirMonitoring() {

    //Create notify process
    int inotify_fd = inotify_init();
    if (inotify_fd == -1) {
        debugLog("Error: Could not initialise inotify");
    }

    //Add watch for folder
    int watch_descriptor = inotify_add_watch(inotify_fd, UPLOAD_DIR, IN_MODIFY | IN_CREATE);
    if (watch_descriptor == -1) {
        debugLog("Error: Could not add directory watch");   
    }

    //set the file descriptor to not block when being read from
    int flags = fcntl(inotify_fd, F_GETFL, 0);
    fcntl(inotify_fd, F_SETFL, flags | O_NONBLOCK);

    return inotify_fd;
}

int checkIfReportsUploaded() {
    FILE* logFile;
    DIR* dir;
    struct dirent* entry;
    bool distribution = false;
    bool manufacturing = false;
    bool sales = false;
    bool warehouse = false;
    time_t currentTime = time(NULL);
    struct tm *timeStruct = localtime(&currentTime);
    char timeBuffer[22];

    dir = opendir(UPLOAD_DIR);

    if (!dir) {
        debugLog("Error opening directory");
        return 1;        
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, DISTRIBUTION_REPORT) == 0) {
            distribution = true;
        } else if (strcmp(entry->d_name, MANUFACTURING_REPORT) == 0) {
            manufacturing = true;
        } else if (strcmp(entry->d_name, SALES_REPORT) == 0) {
            sales = true;
        } else if (strcmp(entry->d_name, WAREHOUSE_REPORT) == 0) {
            warehouse = true;
        }
    }

    logFile = fopen(FILE_LOG_PATH, "a");
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %X", timeStruct);    

    if(!distribution) {
        fprintf(logFile, "[%s] Distribution report was not uploaded\n", timeBuffer);
    }     
    if (!manufacturing) {
        fprintf(logFile, "[%s] Manufacturing report was not uploaded\n", timeBuffer);
    }     
    if (!sales) {
        fprintf(logFile, "[%s] Sales report was not uploaded\n", timeBuffer);
    }
    if (!warehouse) {
        fprintf(logFile, "[%s] Warehouse report was not uploaded\n", timeBuffer);
    }

    fclose(logFile);
    return 0;
}

void checkUploadDirForChanges(int inotify_fd) {
    char event_buffer[EVENT_BUFFER_SIZE];    
    
    ssize_t len = read(inotify_fd, event_buffer, EVENT_BUFFER_SIZE);

    if (len != -1) {
        struct inotify_event *event = (struct inotify_event *)event_buffer;

        if (event->mask & (IN_CREATE)) {
            debugLog("File Created");            
            logChanges(event);
        }

        if (event->mask & (IN_MODIFY)) {
            debugLog("File Modified");
            logChanges(event);
        }
    }       
}