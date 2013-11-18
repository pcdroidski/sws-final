#include "parser.h"

t_httpreq *parse(char *input) {
    char *inputdup, *token, *subtoken;
    t_httpreq *req;

    inputdup = strdup(input);
    req = (t_httpreq*)malloc(sizeof(t_httpreq));

    /* If left unspecified, the req uses HTTP/0.9 */
    req->version = strdup("0.9");
    req->versionlen = 3;
    req->valid = 1;

    /* Parse the HTTP method */
    token = strtok(inputdup, " ");
    if (token == NULL) {
        req->valid = 0;
    } else {
        req->method = token;
        req->methodlen = strlen(token);
        req->method_ident = ident_method(token);
    }

    /* Parse the request URL */
    token = strtok(NULL, " ");
    if (token == NULL) {
        req->valid = 0;
    } else {
        req->url = token;
        req->urllen = strlen(token);
    }

    /* Parse the HTTP version */
    token = strtok(NULL, " ");
    if (token != NULL) {
        if (strncmp(token, "HTTP/", 5) != 0 ||    /* must start with HTTP/ */
            strlen(token) < 8 ||                  /* must have a length of 8 */
            strchr(token+6, '.') == NULL) {       /* version must have a '.' */
            req->valid = 0;
        } else {
            token += 5;

            /* Parse major version */
            subtoken = strtok(token, ".");
            req->majorversion = atoi(subtoken);

            /* Parse minor version */
            subtoken = strtok(NULL, ".");
            if (subtoken == NULL) {
                req->valid = 0;
            } else {
                req->minorversion = atoi(subtoken);

                sprintf(req->version, "%d.%d",
                        req->majorversion, req->minorversion);
                req->versionlen = strlen(req->version);
            }
        }
    }

    free(inputdup);
    return req;
}

t_methodtype ident_method(char *method) {
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

int main(int argc, char **argv) {
    t_httpreq *req;

    req = parse(argv[1]);

    printf("valid: %d\n", req->valid);
    printf("method: %s\n", req->method);
    printf("methodlen: %d\n", req->methodlen);
    printf("method_ident: %d\n", req->method_ident);
    printf("url: %s\n", req->url);
    printf("urllen: %d\n", req->urllen);
    printf("version: %s\n", req->version);
    printf("major: %d\n", req->majorversion);
    printf("minor: %d\n", req->minorversion);

    free(req);
}
