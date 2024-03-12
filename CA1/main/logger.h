void debugLog(char* logString);
void debugLogInt(int number);
void changeLog();
void checkDirForChanges(int inotify_fd);
int setupDirMonitoring();
