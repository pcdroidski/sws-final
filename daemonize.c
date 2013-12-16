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
daemonize(const char *cmd, const char *webroot)
{
    int fdnull, i;

    /* Variables */
	pid_t process_id;
	struct rlimit r_limit;
	struct sigaction sigac;

	/* Clear the file creation mask */
	umask(0);

	/* Fetch maximum number of file descriptors for the given system */
	if (getrlimit(RLIMIT_NOFILE, &r_limit) < 0){
		fprintf(stderr,"%s unable to fetch file limit", cmd);
        exit(0);   
	}
	
    /* Begin forking */
	if ((process_id = fork()) < 0){
		fprintf(stderr,"%s fork failed", cmd);
		exit(1);
	} else if(process_id != 0){
		exit(0);
    }
    
    /* Set session ID */
	setsid();
        
    /* Initiate signal handling */
	sigac.sa_handler = SIG_IGN;
	sigemptyset(&sigac.sa_mask);
	sigac.sa_flags = 0;
    
	if (sigaction(SIGHUP, &sigac, NULL) < 0){
		fprintf(stderr,"%s cannot ignore SIGHUP", cmd);
		exit(1);
	}

	if ((process_id = fork()) < 0){
		fprintf(stderr,"%s fork failed", cmd);
		exit(1);
	} else if(process_id != 0){
        exit(0);        
    }

	/* Change the current working directory to the root */
	if(chdir(webroot) < 0){
		fprintf(stderr, "Cannot change directory to %s\n", webroot);
		exit(0);
	}

	/* Close FDs */
	if (r_limit.rlim_max == RLIM_INFINITY){
		r_limit.rlim_max = 1024;
	}

	for (i = 0; i < r_limit.rlim_max; i++){
		close(i);
	}

	/* Set the FDs of stdin, stdout and stderr to dev/null */
    close(fileno(stdin));
	fdnull = open("dev/null", O_RDWR);
    dup2(fdnull, fileno(stdout));
    dup2(fdnull, fileno(stderr));

	openlog(cmd, LOG_CONS, LOG_DAEMON);
    /* Commenting for Issue #21
	if (fd0 != 0 || fd1 != 1 || fd2 != 2){
		syslog(LOG_ERR, "File descriptors unexpected %d %d %d", fd0, fd1, fd2);
		exit(1);
	}
    */
    
}
