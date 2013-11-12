#ifndef __NET_H__
#define __NET_H__

#include <sys/socket.h>

#define bool char
#define TRUE 1
#define FALSE 0

bool debug;

void
run_server(struct sockaddr_in *);

void
handle_connection(int);

void 
sig_handler(int);

#endif
