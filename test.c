#include "string.h"

#include "parser.h"

void
dump_reqinfo(t_httpreq *);

int
main(int argc, char **argv)
{
    int code;

    t_httpreq *req;
    req = parse(argv[1]);
    dump_reqinfo(req);

    /* Return 0 when OK, 1 on error */
    code = 1 - req->valid;
    free(req);

    return code;
}

void
dump_reqinfo(t_httpreq *req)
{
    printf("valid: %s\n", req->valid ? "VALID" : "INVALID");
    printf("method: %s\n", req->method);
    printf("methodlen: %d\n", req->methodlen);
    printf("method_ident: %d\n", req->method_ident);
    printf("url: %s\n", req->url);
    printf("urllen: %d\n", req->urllen);
    printf("version: %s\n", req->version);
    printf("major: %d\n", req->majorversion);
    printf("minor: %d\n", req->minorversion);
}
