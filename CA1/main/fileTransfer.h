#include <time.h>

int moveAllReports(char* sourceDir, char* targetDir);
int copyFile(char* sourcePath, char* targetPath);

int backupFiles(char* sourceDir, char* targetDir);

int lockDir(const char* dirPath);
int unlockDir(const char* dirPath);

time_t getTransferTime();
time_t getBackupTime();
void transferAndBackupIfTime(time_t* transferTime);

void advanceDay(struct tm* timeStruct);

void makePathStr(char* outputPath, char* dirPath, char* fileName);