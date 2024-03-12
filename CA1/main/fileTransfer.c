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

#define SECS_IN_DAY 86400

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

void transferIfTime(time_t* transferTime) {
    time_t now;

    time(&now);
    
    if(difftime(*transferTime, now) == 0) {            
        lockDir(UPLOAD_DIR);
        lockDir(REPORT_DIR);
        lockDir(BACKUP_DIR);
        moveAllReports(UPLOAD_DIR, REPORT_DIR);
        unlockDir(UPLOAD_DIR);
        unlockDir(REPORT_DIR);
        unlockDir(BACKUP_DIR);
        *transferTime += SECS_IN_DAY; //advance transfer time by 1 day
    }
}