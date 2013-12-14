#include "serve.h"

void
set_status( t_httpreq *req, t_httpresp *resp)
{
    if (!req->valid) {
        //400
        resp->status = HTTP_BAD_REQUEST;

    } else if ( req->majorversion == 0 && req->minorversion == 9 ) {
        /* Handle HTTP/0.9 Requests */
        switch (req->method_ident) {
        case HTTP_GET:
            //200
            resp->status = HTTP_OK;
            break;
        default:
            //400
            resp->status = HTTP_BAD_REQUEST;
        }
    } else if (req->majorversion == 1 && req->minorversion == 0) {
        /* Handle HTTP/1.0 Requests */
        switch (req->method_ident) {
        case HTTP_GET:
        case HTTP_HEAD:
            //200
            resp->status = HTTP_OK;
            break;
        case HTTP_POST:
            //501
            resp->status = HTTP_NOT_IMPLEMENTED;
            break;
        default:
            //400
            resp->status = HTTP_BAD_REQUEST;
        }
    } else if (req->majorversion == 1 && req->minorversion == 1) {
        /* Handle HTTP/1.1 Requests */
        switch (req->method_ident) {
        case HTTP_GET:
        case HTTP_HEAD:
        case HTTP_POST:
            //505
            resp->status = HTTP_VERSION_NOT_SUPPORTED;
            break;
        default:
            //400
            resp->status = HTTP_BAD_REQUEST;
        }
    } else {
        /* Handle other HTTP versions */
        //505
        resp->status = HTTP_VERSION_NOT_SUPPORTED;
    }
}
