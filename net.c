#include <sys/unistd.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "net.h"
#include "parser.h"
#include "response_builder.h"
#include "serve.h"
#include "path.h"

#define MSGBUFSZ 1024
#define MAX_CONNECTIONS 20

char CGI_root[4096];
int g_msgsock;
int sock;

void
run_server(char *address, int port)
{
    /* Setup CGI - if avail */
    if(flags_c == 1){
        if (cgi_dir == NULL){
            if (debug){
               fprintf(stderr,"CGI root directory is missing \n");
            }
            exit(1);
        }

        if (realpath(cgi_dir, CGI_root) == NULL){
            if (debug){
               fprintf(stderr,"CGI root directory error. Check the CGI dir provided \n");
            }
            exit(1);        
        }

        
        struct stat cgi_buf;
        if (stat(CGI_root, &cgi_buf) == -1){
            if (debug){
               fprintf(stderr,"CGI root stat error \n");
            }
            exit(1);
        }

        if (!S_ISDIR(cgi_buf.st_mode)){
            if (debug){
               fprintf(stderr,"CGI root must not be a directory \n");
            }

            exit(1);
        }
        
        if (debug)
            printf("CGI root set to: %s \n", CGI_root);
   }
    
    int msgsock;

    if ((msgsock = bind_socket(address, port)) == -1) {
        printf("Cannot start the server \n");
        exit(1);
    }

    /* - - Start Server - - */

    int clientFD;
    do {
        struct addrinfo hint, *result;
        memset(&hint, 0, sizeof(struct addrinfo));

        /* Set up signal handler to reap children */
        signal(SIGCHLD, &sig_handler);

        /* Set up signal handler to shut down */
        signal(SIGTERM, &sig_handler);
        signal(SIGINT, &sig_handler);

        /* Initiate the variables */
        result = (struct addrinfo *) malloc(sizeof(struct addrinfo));
        hint.ai_family = AF_UNSPEC;

        /* specified, check type*/
        if (address != NULL)
            getaddrinfo(address, 0, &hint, &result);

        /* Not specified, use IPv6 by default.    */
        if (address == NULL)
            result -> ai_family = AF_INET6;


        // If it is a IPv6, then using IPv6 address family to listen.
        if (result -> ai_family == AF_INET6) {
            struct sockaddr_in6 remote;
            socklen_t receive = INET6_ADDRSTRLEN;
            memset(&remote, 0, sizeof(struct sockaddr_in6));

            if ((clientFD = accept(msgsock, (struct sockaddr *)&remote,
                &receive)) < 0) {
                fprintf(stderr, "[ERROR]accept error\n");
                exit(1);
            }

            // // Get the incomming request address from client
            // if (inet_ntop(AF_INET6, &remote.sin6_addr.s6_addr, incomming,
            //     INET6_ADDRSTRLEN) == NULL){
            //     perror(strerror(errno));
            // }
        }

        // If it is a IPv4, using IPv4 address family.
        if (result -> ai_family == AF_INET) {
            struct sockaddr_in remote;
            socklen_t receive = INET_ADDRSTRLEN;
            memset(&remote, 0, sizeof(struct sockaddr_in));

            if ((clientFD = accept(msgsock, (struct sockaddr *)&remote,
                &receive)) < 0) {
                fprintf(stderr,"[ERROR]accept error\n");
                perror(strerror(errno));
                exit(1);
            }

            // // // Get the incomming request address
            // if (inet_ntop(AF_INET, &remote.sin_addr.s_addr,
            //     incomming,INET_ADDRSTRLEN) == NULL) {
            //     perror(strerror(errno));
            // }
        }

        handle_connection(clientFD);
    } while (TRUE);

}

int
sock_readline(int sockfd, char *buf, int sz)
{
    int i;
    for (i = 0; read(sockfd, &buf[i], 1) > 0 && buf[i] != '\n'; i++);
    buf[i-1] = '\0';
    return i;
}

void
handle_connection(int msgsock)
{
    int pid, port;
    struct sockaddr_storage client;
    struct sockaddr_in *addr4;
    struct sockaddr_in6 *addr6;
    socklen_t len;
    char buf[INET6_ADDRSTRLEN];
    char msg[MSGBUFSZ];
    int nbytes;
    t_httpreq *req;
    t_httpresp *res;
    struct sigaction timeout;
    struct itimerval timer;
    struct tm headertm;

    if ((pid = fork()) == 0) {
        /* Execute child code */
        if (debug)
            printf("Child executing with pid=%d\n", getpid());

        if (getpeername(msgsock, (struct sockaddr *) &client, &len) == 0) {

            /* Handle IPv4 and IPv6 connections */
              if (client.ss_family == AF_INET) {
                addr4 = (struct sockaddr_in *) &client;
                port = ntohs(addr4->sin_port);
                inet_ntop(AF_INET, &addr4->sin_addr, buf, sizeof(buf));
            } else {
                addr6 = (struct sockaddr_in6 *) &client;
                port = ntohs(addr6->sin6_port);
                inet_ntop(AF_INET6, &addr6->sin6_addr, buf, sizeof(buf));
            }

            /* TODO Logging connections */
            if (debug)
                printf("Connection from %s:%d\n", buf, port);

            /* Store the socket fd globally so we can write to it in a
             * signal handler */
            g_msgsock = msgsock;

            /* Read one line from the client and parse it */
            if ((nbytes = sock_readline(msgsock, msg, MSGBUFSZ)) >= 0) {
                req = parse(msg);
                res = init_response();

                set_status(req, res);

                printf("%s: %d\n", buf, res->status);
            } else {
                perror("read socket");
            }
            
            // The stuff in the middle - this is where we check for timeouts
            memset(&timeout, 0, sizeof (timeout));
            timeout.sa_handler = &sig_handler;
            sigaction(SIGALRM, &timeout, NULL);

            timer.it_value.tv_sec = CONNECTION_TIMEOUT_SECONDS;
            timer.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &timer, NULL);

            /* headers! read a line and do nothing with it */
            while ((nbytes = sock_readline(msgsock, msg, MSGBUFSZ)) >= 0) {
                if (strncmp(msg, "If-Modified-Since: ", 19) == 0) {
                    strptime(&msg[19], "%a, %d %b %Y %T %z", &headertm);
                    req->ifmodifiedsince = mktime(&headertm);
                } else if (msg[0] == '\0') {
                    break;
                }
            }

            /** Handle logic for CGI vs other request */            
            if (flags_c == 1 && strncmp(req->url, "cgi-bin/", 8) == 0){
                response_set_cgi(res, req->url, CGI_root, req->ifmodifiedsince);
                
            } else {
                response_set_file(res, req->url, req->ifmodifiedsince);
            }    

            finalize_response(res);
            write_response(res, msgsock); 

        } else {
            perror("getpeername");
        }

        if (debug)
            printf("Child finished executing, pid=%d\n", getpid());

        /* Close the socket connection */
        if (close(msgsock) != 0)
            perror("close");

        exit(0);
    } else {
        /* Execute parent code */

        /* Close the socket connection */
        if (close(msgsock) != 0)
            perror("close");
    }
}


int
bind_socket(char *address, int port) {
    int sock;

    if(flags_i == 1) {
        struct addrinfo baseInfo, *addressInfo = NULL;

        memset(&baseInfo, 0, sizeof(struct addrinfo));

        baseInfo.ai_family = AF_UNSPEC;
        getaddrinfo(address, 0, &baseInfo, &addressInfo);

        // If address is IPv4, use that family of sockets
        if(addressInfo->ai_family == AF_INET) {

              // Create the socket
              if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("Error opening stream socket");
                  exit(1);
              }

              struct sockaddr_in server;
              server.sin_family = AF_INET;
              server.sin_port = htons(port);

            /* Convert address to a network format */
              inet_pton(AF_INET, address, &server.sin_addr.s_addr);

            // Binding the socket to the port number(default is 8080)
            if (bind(sock, (struct sockaddr *)&server, sizeof(server))) {
                printf("Cannot bind socket. Either port is in use or given IP Address is in use. \n");
                exit(1);
            }
        }

        // If address is IPv6, use that family of sockets
        if (addressInfo->ai_family == AF_INET6) {

            // Create the socket
            if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
                printf("Socket Error");
                exit(1);
            }

            struct sockaddr_in6 server;
            server.sin6_family = AF_INET6;
            server.sin6_port = htons(port);

            /* Convert address to a network format */
            inet_pton(AF_INET6, address, &server.sin6_addr);

            // Binding the socket to the port number(default is 8080)
            if (bind(sock, (struct sockaddr *)&server, sizeof(server))) {
                printf("Cannot bind socket. Either port is in use or given IP Address is in use. \n");
                exit(1);
            }
        }
    } else {
        // Create the socket
        if ((sock = socket(AF_INET6,SOCK_STREAM,0)) < 0) {
            printf("Socket Error");
            exit(1);
        }

        struct sockaddr_in6 server;
        server.sin6_family = AF_INET6;
        server.sin6_port = htons(port);
        server.sin6_addr = in6addr_any;

        if (debug)
            printf("Listening on all IPv4 and IPv6 addresses\n");

        // Binding the socket to the port number(default is 8080)
        if (bind(sock, (struct sockaddr *)&server, sizeof(server))){
            printf("Cannot bind socket. Either port is in use or given IP Address is in use. \n");
            exit(1);
        }
    }

    // Start listening on Socket. Max connections set to 20 for now.
    if (listen(sock, MAX_CONNECTIONS) == -1){
        printf("Socket listening error \n");
        exit(1);
    }

    return sock;
}

void
sig_handler(int signo)
{
    pid_t pid;
    t_httpresp *res;

    if (signo == SIGCHLD) {
        pid = wait(0);
        if (debug) {
            puts("Child process ended");
            if (pid > 0)
                printf("Reaped child process with pid=%d\n", pid);
            else
                printf("Child process returned error result; %d; %s\n",
                        errno, strerror(errno));
        }
    } else if (signo == SIGALRM) {
        res = init_response();
        if (res == NULL) {
            fprintf(stderr, "malloc generating response error in timeout \n");
            exit(1);
        }

        res->status = HTTP_TIMEOUT;
        finalize_response(res);
        write_response(res, g_msgsock);
        close(g_msgsock);
        exit(0);
    } else if (signo == SIGTERM || signo == SIGINT) {
        puts("Shutting down...");
        if (close(sock) != 0)
            perror("close");
        exit(0);
    }
}
