#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "daemonConfig.h"

void debugLog(char* logString) {
    FILE* logFile;

    logFile = fopen(LOG_DIR, "a");

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