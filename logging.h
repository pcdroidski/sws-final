#include <stdlib.h>
#include <stdio.h>
#include "system.h"     
#include "log.h"

FILE *file;


void logSetup(char *log_file){
    file = fopen(log_file, "a");

    if (file == NULL) {
        return
    }
}

void logMessage (char *message)
{
    fputs(message, file);
    fclose(file);
}


void logInfo (char *message, char *log_file)
{
    logSetup(log_file);
    logMessage("[INFO]");
    logMessage(message);
    logMessage("\n");
    
    if (file)
        fclose(file);
}

void logError (char *message, char *log_file)
{
    logSetup(log_file);
    logMessage("[ERROR]");
    logMessage(message);
    logMessage("\n");
    
    if (file)
        fclose(file);
}
