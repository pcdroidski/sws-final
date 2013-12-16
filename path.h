#ifndef __PATH_H__
#define __PATH_H__

#include <sys/types.h>
#include <pwd.h>
#include <uuid/uuid.h>
#include "parser.h"

#define PATH_MAX 512
#define NAME_SZ  64

char *path_normalize(char *);

#endif
