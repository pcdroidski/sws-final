#ifndef __NET_H__
#define __NET_H__

#include <sys/socket.h>

#define bool char
#define TRUE 1
#define FALSE 0

#define CONNECTION_TIMEOUT_SECONDS 10

extern int flags_i;
extern char *cgi_dir;

bool debug;
bool logging;

void
run_server(char *address, int port);

int
sock_readline(int sockfd, char *, int);

void
handle_connection(int);

int
bind_socket(char *address, int port);

void
sig_handler(int);

#endif
