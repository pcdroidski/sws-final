#include "string.h"

#include "parser.h"
#include "response_builder.h"

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

    /* Not an effective test, but spits out some response to stdout */
    t_httpresp *resp;
    resp = init_response();
    resp->status = HTTP_OK;

    resp->content = strdup("This is the response body.");

    finalize_response(resp);
    write_response(resp, 1);
    free_response(resp);

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
