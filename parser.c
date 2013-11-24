#include "parser.h"

#define MAX_VERSION_LENGTH      6
#define DEFAULT_VERSION     "0.9"

t_httpreq *
parse(char *input)
{
    char *inputdup, *token, *subtoken;
    char vbuf[MAX_VERSION_LENGTH];
    int n; /* For storing temporary value, i.e return codes */
    t_httpreq *req;

    inputdup = strdup(input);
    req = (t_httpreq*)malloc(sizeof(t_httpreq));

    /* If left unspecified, the req uses HTTP/0.9 */
    req->version = strdup(DEFAULT_VERSION);
    req->versionlen = 3;
    req->valid = 1;

    /* Parse the HTTP method */
    if ((token = strtok(inputdup, " ")) == NULL) {
        req->valid = 0;
    } else {
        /* Note: method may be null if strdup fails */
        req->method = strdup(token);
        req->methodlen = strlen(token);
        req->method_ident = ident_method(token);
    }

    /* Parse the request URL */
    if ((token = strtok(NULL, " ")) == NULL) {
        req->valid = 0;
    } else {
        /* Note: url may be null if strdup fails */
        req->url = strdup(token);
        req->urllen = strlen(token);
    }

    /* Parse the HTTP version */
    if ((token = strtok(NULL, " ")) != NULL) {
        if (strncmp(token, "HTTP/", 5) != 0 ||  /* must start with HTTP/ */
            strlen(token) < 8 ||                /* must have a length of 8 */
            strchr(token+6, '.') == NULL) {     /* version must have a '.' */

            req->valid = 0;
        } else {
            token += 5;

            /* Parse major version */
            subtoken = strtok(token, ".");
            req->majorversion = atoi(subtoken);

            /* Parse minor version */
            if ((subtoken = strtok(NULL, ".")) == NULL) {
                req->valid = 0;
            } else {
                req->minorversion = atoi(subtoken);

                if ((n = sprintf(vbuf, "%d.%d",
                    req->majorversion, req->minorversion)) > 0) {

                    /* Free default version that was allocated */
                    free(req->version);

                    /* Note: Version may be null if strdup fails */
                    req->version = strdup(vbuf);
                    req->versionlen = strlen(vbuf);
                }
            }
        }
    }

    free(inputdup);
    return req;
}

t_methodtype
ident_method(char *method)
{
    if (strcmp(method, "GET") == 0) {
        return HTTP_GET;
    } else if (strcmp(method, "HEAD") == 0) {
        return HTTP_HEAD;
    } else if (strcmp(method, "POST") == 0) {       /* POST is later */
        return HTTP_POST;
    } else {
        return HTTP_OTHER;
    }
}
