#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#include "daemonConfig.h"
#include "logger.h"
#include "fileTransfer.h"

int backupFiles(char* sourceDir, char* targetDir) {
    DIR* dir;
    struct dirent* entry;
    char sourcePath[strlen(sourceDir) + FILE_NAME_MAX_LEN];     
    char targetPath[strlen(targetDir) + FILE_NAME_MAX_LEN];     

    lockDir(sourceDir);
    lockDir(targetDir);

    dir = opendir(sourceDir);

    if (!dir) {
        debugLog("Error opening directory");
        return 1;        
    }

    while ((entry = readdir(dir)) != NULL) {
        makePathStr(sourcePath, sourceDir, entry->d_name);
        makePathStr(targetPath, targetDir, entry->d_name);
        copyFile(sourcePath, targetPath);
    }

    unlockDir(sourceDir);
    unlockDir(targetDir);

    return 0;
}


void makePathStr(char* outputPath, char* dirPath, char* fileName) {
    strcpy(outputPath, dirPath);
    strcat(outputPath, "/");
    strcat(outputPath, fileName);
}

int moveAllReports(char* sourceDir, char* targetDir) {    
    lockDir(sourceDir);
    lockDir(targetDir);
    
    char sourcePath[strlen(sourceDir) + FILE_NAME_MAX_LEN];     
    char targetPath[strlen(targetDir) + FILE_NAME_MAX_LEN];     

    makePathStr(sourcePath, sourceDir, DISTRIBUTION_REPORT); 
    makePathStr(targetPath, targetDir, DISTRIBUTION_REPORT);
    copyFile(sourcePath, targetPath);
    remove(sourcePath);

    makePathStr(sourcePath, sourceDir, MANUFACTURING_REPORT); 
    makePathStr(targetPath, targetDir, MANUFACTURING_REPORT);
    copyFile(sourcePath, targetPath);
    remove(sourcePath);

    makePathStr(sourcePath, sourceDir, SALES_REPORT); 
    makePathStr(targetPath, targetDir, SALES_REPORT);
    copyFile(sourcePath, targetPath);
    remove(sourcePath);

    makePathStr(sourcePath, sourceDir, WAREHOUSE_REPORT); 
    makePathStr(targetPath, targetDir, WAREHOUSE_REPORT);
    copyFile(sourcePath, targetPath);
    remove(sourcePath);

    unlockDir(sourceDir);
    unlockDir(targetDir);
}

int copyFile(char* sourcePath, char* targetPath) {
    FILE *sourceFile, *destinationFile;
    char buffer[REPORT_SIZE];
    size_t bytesRead;
        
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



