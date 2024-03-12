#include <time.h>

int moveAllReports(char* sourceDir, char* targetDir);
int moveReport(char* sourceDir, char* targetDir, char* reportName);

int backupDir(char* sourceDir, char* targetDir);

int lockDir(const char* dirPath);
int unlockDir(const char* dirPath);

time_t getTransferTime();
time_t getBackupTime();
void transferIfTime(time_t* transferTime);

void advanceDay(struct tm* timeStruct);