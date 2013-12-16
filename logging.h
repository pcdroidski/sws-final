#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/unistd.h>
#include <fcntl.h>


FILE *file;

bool logSetup(char *log_file);

void logMessage (char *message);

void logInfo (char *message, char *log_file);

void logError (char *message, char *log_file);