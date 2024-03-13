#include <signal.h>

#include "daemonConfig.h"
#include "fileTransfer.h"
#include "logger.h"

void handleSignal(int signo) {
    if (signo == SIGUSR1) {             
        moveAllReports(UPLOAD_DIR, REPORT_DIR);
        debugLog("Transferred files from upload to reporting.");
    } else if (signo == SIGUSR2) {
        backupFiles(REPORT_DIR, BACKUP_DIR);
        debugLog("Backed up files.");
    }
}