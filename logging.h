#include <stdlib.h>
#include <stdio.h>

FILE *file;

void logSetup(char *log_file);

void logMessage (char *message);

void logInfo (char *message, char *log_file);

void logError (char *message, char *log_file);