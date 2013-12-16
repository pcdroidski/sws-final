#include "path.h"

/**
 * Normalizes a given path
 * Returns the path with ~'s resolved
 * Returns NULL if the path exceeds the webroot (i.e. /../../../)
 */
char *path_normalize(char *path) {
    int i, level;
    char *realpath, username[NAME_SZ];
    struct passwd *userent;

    level = 0;

    if ((realpath = (char *)malloc(PATH_MAX)) == NULL) {
        printf("Failed to malloc.\n");
        exit(1);
    }

    username[0] = realpath[0] = '\0';

    if (path == NULL) {
        return NULL;
    } else if (path[0] == '/') {
        path++;
    }

    if (path[0] == '~') {
        for (i = 1; path[i] != '/' && path[i] != '\0'; i++) ;
        strncat(username, path+1, i-1);

        path += i;

        if ((userent = getpwnam(username)) == NULL) {
            return NULL;
        }
        strcat(realpath, userent->pw_dir);
    }

    for (i = 0; path[i] != '\0'; ) {
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

    strcat(realpath, path);
    return realpath;
}
