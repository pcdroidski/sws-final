#include <fcntl.h>
#include <sys/resource.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>


int 
daemonize(const char *cmd)
{
    /* File descriptors */
    int fd0, fd1, fd2;

    /* Variables */
	pid_t process_id;
	struct rlimit r_limit;
	struct sigaction sigac;

	/* Clear the file creation mask */
	umask(0);

	/* Fetch maximum number of file descriptors for the given system */
	if (getrlimit(RLIMIT_NOFILE, &r_limit) < 0){
		fprintf(stderr,"%s unable to fetch file limit", cmd);
        return 1;   
	}
	
    /* Begin forking */
	if ((process_id = fork()) < 0){
		fprintf(stderr,"%s fork failed", cmd);
		return 1;
	} else if(process_id != 0){
		return 1;
    }
    
    /* Set session ID */
	setsid();
        
    /* Initiate signal handling */
	sigac.sa_handler = SIG_IGN;
	sigemptyset(&sigac.sa_mask);
	sigac.sa_flags = 0;
    
	if (sigaction(SIGHUP, &sigac, NULL) < 0){
		fprintf(stderr,"%s cannot ignore SIGHUP", cmd);
		return 1;
	}

	if ((process_id = fork()) < 0){
		fprintf(stderr,"%s fork failed", cmd);
		return 1;
	} else if(process_id != 0){
        return 1;
    }

	/* Change the current working directory to the root */
	if(chdir("/") < 0)
		fprintf(stderr, "Cannot change directory to / \n");

	/* Close FDs */
	if (r_limit.rlim_max == RLIM_INFINITY){
		r_limit.rlim_max = 1024;
	}

	int i;
	for (i = 0; i < r_limit.rlim_max; i++){
		close(i);
	}

	/* Set the FDs of stdin, stdout and stderr to dev/null */
	fd0 = open("dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	fd0 += 0;
	fd1 += 0;
	fd2 += 0;
    
    return 0;
}