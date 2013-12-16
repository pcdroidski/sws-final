#include <fcntl.h>
#include <sys/resource.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>



void 
daemonize(const char *cmd, const char *webroot);
