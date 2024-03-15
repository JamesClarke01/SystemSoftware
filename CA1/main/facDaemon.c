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
#include "daemonSetup.h"

#define SECS_IN_DAY 86400

int main() {
    time_t transferTime, uploadTime, now;
    int inotify_fd;

    if (initDaemon() == 1) {
        errorLog("Starting Daemon");
        return 1;
    }

    transferTime = getTransferTime();
    uploadTime = getUploadTime();
    
    inotify_fd = setupDirMonitoring();

    while(1) {                
        sleep(1);        
        
        time(&now);                
        if(difftime(transferTime, now) == 0) {            
            moveAllReports(UPLOAD_DIR, REPORT_DIR);
            backupFiles(REPORT_DIR, BACKUP_DIR);
            transferTime += SECS_IN_DAY; 
        } else if(difftime(uploadTime, now) == 0) {            
            checkIfReportsUploaded();
            uploadTime += SECS_IN_DAY; 
        }
    
        checkUploadDirForChanges(inotify_fd);
    }
}

