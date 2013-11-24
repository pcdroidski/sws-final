#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"

/* Flags */
int flags_c = 0;
int flags_d = 0;
int flags_h = 0;
int flags_i = 0;
int flags_l = 0;
int flags_p = 0;

/* Variables */
int port = 8080;
char *address;
char *dir = NULL;
char *log_file = NULL;
char *cgi_dir = NULL;


void
usage(int);

void setup_options(int arc, char *argv[]);


/** Fetch all options passed in arv */
void
setup_options(int argc, char *argv[])
{
    char opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
        case 'c':
            flags_c = 1;
            cgi_dir = strdup(optarg);
            break;
        case 'd':
            debug = TRUE;
            break;
        case 'h':
            usage(0);
            break;
        case 'i':
            flags_i = 1;
            address = optarg;
            break;
        case 'l':
            flags_l = 1;
            log_file = strdup(optarg);
            logging = TRUE;
            break;
        case 'p':
            flags_p = 1;
            port = atoi(optarg);
            errno = 0;
            if (errno == EINVAL || errno == ERANGE) {
                perror("parsing port number");
                exit(1);
            } else if (port <= 0) {
                printf("Can't bind to port: '%s'\n", optarg);
                exit(1);
            }
            break;
        }
    }

    dir = argv[optind];

    if (debug) {
      if (flags_c) {
        printf("CGI execution allowed from dir: %s \n", cgi_dir);
      }
      if (flags_i) {
        printf("IP Address binded to: %s \n", address);
      }
      if (flags_l) {
        printf("Logging all requests to: %s \n", log_file);
      }
      if (flags_p) {
        printf("Configured to use port: %d \n", port);
      }
    }
}

int
main(int argc, char **argv)
{
    debug = FALSE;
    logging = FALSE;
    //logfilename = NULL;

    /* Read program options */
    setup_options(argc, argv);

    if (debug)
        printf("Configured to use port: %d\n", port);

    run_server(address, port);
    return 0;
}

void
usage(int code)
{
    puts("Usage");
    exit(code);
}

