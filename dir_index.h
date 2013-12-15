#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#define MAX_DIR_PAGE   4096
#define HTML_HEAD       "<!DOCTYPE html><head></head><body><h1>The contents of this directory : </h1><br />"
#define HTML_TAIL       "</body></html>"
#define ANCHOR_LEN      22


void
makeIndex( char * const *dir, char *index );
