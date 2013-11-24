#ifndef __NET_H__
#define __NET_H__

#include <sys/socket.h>

#define bool char
#define TRUE 1
#define FALSE 0

extern int flags_i;

bool debug;

void
run_server(char *address, int port);

void
handle_connection(int);

int
bind_socket(char *address, int port);

void
sig_handler(int);

#endif
