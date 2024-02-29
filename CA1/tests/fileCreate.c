#include <stdio.h>


int main() {
    FILE *fp;
    
    fp = fopen("//home/SystemSoftware/CA1/daemon.pid", "w");
    if(fp != NULL) {        
        printf("File open suceeded\n");
        fprintf(fp, "%d", 1);
        fclose(fp);
    } else {
        printf("File not opened successfully\n");
    }
}
