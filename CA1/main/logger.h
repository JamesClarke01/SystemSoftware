void debugLog(char* logString);
void debugLogInt(int number);
void errorLog(char* errorString);
void checkUploadDirForChanges(int inotify_fd);
int setupDirMonitoring();
int checkIfReportsUploaded();
