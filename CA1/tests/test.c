#include <stdio.h>
#include <time.h>
#include "daemon.h"

int main() {
    struct tm backupTime;
    time_t now;

    time(&now); //set to the current time

    backupTime = *localtime(&now); //set to current date

    //set backup time
    backupTime.tm_hour = TRANSFER_HR; 
    backupTime.tm_min = TRANSFER_MIN;
    backupTime.tm_sec = TRANSFER_SEC;

    printf("Now: %ld\n", now);
    printf("Backup time: %ld\n", mktime(&backupTime));    
    printf("Time diff: %f\n", difftime(mktime(&backupTime), now));

    return 0;
}


