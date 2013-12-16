#include "path.h"

/**
 * Normalizes a given path
 * Returns the path joined with the current webroot (defined currently as CWD)
 * Returns NULL if the path exceeds the webroot (i.e. /../../../)
 */
char *path_normalize(char *path) {
    int i, level;

    i = level = 0;

    if (path == NULL) {
        return NULL;
    } else if (path[0] == '/') {
        i++;
    }

    for (; path[i] != '\0';) {
        if (strcmp(&path[i], "..") == 0) {
            level--;
            i += 2;
        } else if (strncmp(&path[i], "../", 3) == 0) {
            level--;
            i += 3;
        } else if (strncmp(&path[i], "./", 2) == 0) {
            i += 2;
        } else {
            while (path[i] != '/' && path[i] != '\0') i++;
            level++;
            while (path[i] == '/') i++;
        }

        if (level < 0) {
            return NULL;
        }
    }

    /* Always return a relative path */
    if (path[0] == '/')
        return path + 1;

    return path;
}
