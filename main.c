#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"

char *cgi_dir = NULL;

void
usage(int);

int
main(int argc, char **argv)
{
        struct sockaddr_in server;
        char opt;
        //char *logfilename;
        int port;

        debug = FALSE;
        //logfilename = NULL;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(8080);

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
                        server.sin_addr.s_addr = inet_addr(optarg);
                        break;
                case 'l': /* Log to the given file */
                        //logfilename = strdup(optarg);
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
                        } else {
                                server.sin_port = htons(port);
                        }
                        break;
                }
        }

        if (debug)
                printf("Configured to use port: %d\n", ntohs(server.sin_port));
        
        run_server(&server);

        return 0;
}

void
usage(int code)
{
        puts("Usage");
        exit(code);
}

