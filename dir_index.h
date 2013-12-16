#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#define MAX_DIR_PAGE   4096
#define HTML_HEAD       "<!DOCTYPE html><head></head><body><h3>Directory listing:</h3><ul>"
#define HTML_TAIL       "</ul></body></html>"
#define ANCHOR_LEN      22


void
makeIndex( char *dir, char *index );
