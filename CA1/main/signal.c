#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "daemonConfig.h"
#include "fileTransfer.h"
#include "logger.h"


void restartDaemon() {
    char programPath[100];

    strcpy(programPath, DAEMON_DIR);
    strcat(programPath, "/facDaemon");
    
    execl(programPath, "facDaemon", NULL);

    debugLog("Exec failed");
}


void handleSignal(int signo) {
    if (signo == SIGUSR1) {             
        moveAllReports(UPLOAD_DIR, REPORT_DIR);
        debugLog("Transferred files from upload to reporting.");
    } else if (signo == SIGUSR2) {
        backupFiles(REPORT_DIR, BACKUP_DIR);
        debugLog("Backed up files.");
    } else if (signo == SIGHUP) {
        debugLog("Restarting Daemon");
        restartDaemon();
    }
}