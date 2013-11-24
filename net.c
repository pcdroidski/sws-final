#include <sys/unistd.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"

#define MSGBUFSZ 1024

int sock;

void
run_server(struct sockaddr_in *server)
{
    socklen_t len;
    int msgsock;

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("opening socket");
        exit(1);
    }

    /* Bind to the socket */
    if (bind(sock, (struct sockaddr*) server, sizeof(*server))) {
        perror("binding stream socket");
        exit(1);
    }

    /* DEBUG: Print the actual port number */
    if (debug) {
        /* Find out assigned port number and print it out */
        len = sizeof(*server);
        if (getsockname(sock, (struct sockaddr*) server, &len)) {
            perror("getting socket name");
            exit(1);
        }
        printf("Socket has port #%d\n", ntohs(server->sin_port));
    }

    /* Set up signal handler to reap children */
    signal(SIGCHLD, &sig_handler);

    /* Set up signal handler to shut down */
    signal(SIGTERM, &sig_handler);
    signal(SIGINT, &sig_handler);

    /* Listen on the socket for connections */
    /* TODO Fine-tune the 'backlog' parameter; listen(3) */
    listen(sock, 5);
    do {
        if ((msgsock = accept(sock, 0, 0)) < 0)
            perror("accept");
        else
            handle_connection(msgsock);
    } while (TRUE);
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

            /* Read a single message from the client */
            if ((nbytes = read(msgsock, &msg, MSGBUFSZ-1)) >= 0) {
                msg[nbytes-1] = '\0';
                printf("%s: %s\n", buf, msg);
            } else {
                perror("read socket");
            }

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
    }
}

void
sig_handler(int signo)
{
    pid_t pid;
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
    } else if (signo == SIGTERM || signo == SIGINT) {
        puts("Shutting down...");
        if (close(sock) != 0)
            perror("close");
        exit(0);
    }
}

