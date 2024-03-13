void debugLog(char* logString);
void debugLogInt(int number);
void changeLog();
void checkUploadDirForChanges(int inotify_fd);
int setupDirMonitoring();
int checkIfReportsUploaded();
