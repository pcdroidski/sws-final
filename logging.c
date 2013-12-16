#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include "logging.h"


FILE *file;


bool logSetup(char *log_file){
    file = fopen(log_file, "a");

    if (file == NULL) {
        return false;
    }
    
    return true;
}

void logMessage (char *message)
{
    fputs(message, file);
}


void logInfo (char *message, char *log_file)
{
    if (logSetup(log_file)){
        
        logMessage("[INFO]");
        logMessage(message);
        logMessage("\n");
    
        if (file)
            fclose(file);
    }
}

void logError (char *message, char *log_file)
{
    if (logSetup(log_file)){
        logMessage("[ERROR]");
        logMessage(message);
        logMessage("\n");
    
        if (file)
            fclose(file);
    }
}
