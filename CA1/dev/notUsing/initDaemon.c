#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void startDaemon();
void stopDaemon(); 

int main(int argc, char *argv[] ) {
    if (argc < 2) {
        printf("Invalid number of arguments entered.\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], "start") == 0) {
        startDaemon();
    } else if (strcmp(argv[1], "stop") == 0) {
        stopDaemon();
    }

    return 0;
}

void startDaemon() {
    printf("Starting Daemon...\n");
}

void stopDaemon() {
    printf("Stopping Daemon...\n");
}