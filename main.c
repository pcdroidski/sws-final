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

char *cgi_dir = NULL;

void
usage(int);

int
main(int argc, char **argv)
{
    char opt;
    char *logfilename;
    int port = 8080;
    char *address;

    debug = FALSE;

    /* Read program options */
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
        case 'c': /* Execute CGI files in the given directory */
            cgi_dir = strdup(optarg);
            break;
        case 'd': /* Enter debug mode */
            debug = TRUE;
            break;
        case 'h': /* Print a short usage summary and exit */
            usage(0);
            /* NOTREACHED */
        case 'i': /* Bind to the given IPv4 or IPv6 address */
            address = strdup(optarg);
            break;
        case 'l': /* Log to the given file */
            logfilename = strdup(optarg);
            break;
        case 'p': /* Listen on the given port */
            errno = 0;
            port = atoi(optarg);
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

    if (debug)
        printf("Configured to use port: %d\n", port);

    run_server(address, port);

    run_server(&server);

    return 0;
}

void
usage(int code)
{
    puts("Usage");
    exit(code);
}

