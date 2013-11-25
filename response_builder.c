/*
 * sws - response builder
 *
 * Component responsible for building HTTP
 * response objects and serializing them to
 * an output file descriptor.
 *
 * Author: Nick Smith
 */

#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "response_builder.h"

#define BUFSZ       4096
#define SERVER_NAME     "sws"
#define SERVER_VERSION  "0.1"

char buf[BUFSZ];

t_httpresp *
init_response()
{
    struct utsname host;
    t_httpresp *resp;
    int nchars;

    /* If we can't allocate a response... not much else to do */
    if ((resp = malloc(sizeof(t_httpresp))) == NULL) {
        return NULL;
    }

    /* Intialize the default values */
    resp->headers = NULL;
    resp->nheaders = 0;
    resp->headers_len = 0;
    resp->protocol = strdup("HTTP");
    resp->major_version = 1;
    resp->minor_version = 0;

    /* Print the server name, version and host information */
    if ((nchars = sprintf(buf, "%s %s", SERVER_NAME, SERVER_VERSION)) > 0) {

        /* Try to get more host information */
        if (uname(&host) >= 0) {
            nchars = sprintf(buf + nchars, " %s %s %s %s %s",
                host.sysname, host.nodename, host.release,
                host.version, host.machine);

        }

        /* Set the 'Server' response header */
        response_set_header(resp, HEADER_SERVER, buf);
    }

    return resp;
}

bool
response_set_header(t_httpresp *resp, char *name, char *value)
{
    void *ptr;
    t_httpresp_header *hdr;
    
    /* Sanity checks */
    if (resp == NULL || name == NULL || value == NULL) {
        return false;
    }

    if (resp->headers == NULL || resp->headers_len <= resp->nheaders) {
        if ((ptr = realloc(resp->headers,
                sizeof(t_httpresp_header) *
                (resp->headers_len + 16))) != NULL) {

            resp->headers = (t_httpresp_header *) ptr;
            resp->headers_len += 16;
        } else {
            /* Couldn't allocate room for new header */
            return false;
        }
    }

    /* Select the next header */
    hdr = &(resp->headers[resp->nheaders]);

    if ( (hdr->name  = strdup(name) ) != NULL &&
         (hdr->value = strdup(value)) != NULL) {
        
        /* Increment the number of stored headers */
        resp->nheaders++;
    } else {
        /* Couldn't allocate the header info */
        return false;
    }

    /* Header was added successfully */
    return true;
}

bool
finalize_response(t_httpresp *resp)
{
    time_t t;
    struct tm *local;
    int nchars;

    /* Quick sanity check */
    if (resp == NULL) {
        return false;
    }

    t = time(NULL);
    if ((local = localtime(&t)) != NULL && 
        strftime(buf, BUFSZ, "%a, %d %b %Y %T %z", local) >= 0) {
        
        /* Set the 'Date' response header */
        response_set_header(resp, HEADER_DATE, buf);
    }

    nchars = resp->content != NULL ? strlen(resp->content) : 0;
    if (sprintf(buf, "%d", nchars) > 0) {
        /* Set the 'Content-Length' response header */
        response_set_header(resp, HEADER_CONTENT_LENGTH, buf);
    }

    /* Response is reasdy to be sent */
    return true;
}

void
free_response(t_httpresp *resp)
{
    t_httpresp_header *hdr;
    if (resp == NULL)
        return;

    /* Free the things that are dynamically allocated */
    if (resp->content != NULL)
        free(resp->content);
    if (resp->protocol != NULL)
        free(resp->protocol);

    /* Free the headers */
    if (resp->headers != NULL) {

        /* Free the information in each header */
        while (--(resp->nheaders) >= 0) {
            hdr = &(resp->headers[resp->nheaders]);

            if (hdr->name != NULL)
                free(hdr->name);
            if (hdr->value != NULL)
                free(hdr->value);
        }

        free(resp->headers);
    }
}

bool
write_response(t_httpresp *resp, int fd)
{
    FILE *f;
    int i;
    
    if ((f = fdopen(fd, "a")) != NULL) {
        fprintf(f, "%s/%d.%d   %d", resp->protocol,
            resp->major_version, resp->minor_version,
            resp->status);

        switch (resp->status) {
        case HTTP_OK:
            fputs(" OK\n", f);
            break;
        case HTTP_CREATED:
            fputs(" Created\n", f);
            break;
        case HTTP_BAD_REQUEST:
            fputs(" Bad Request\n", f);
            break;
        case HTTP_FORBIDDEN:
            fputs(" Forbidden\n", f);
            break;
        case HTTP_NOT_FOUND:
            fputs(" Not Found\n", f);
            break;
        case HTTP_METHOD_NOT_ALLOWED:
            fputs(" Method Not Allowed\n", f);
            break;
        case HTTP_TIMEOUT:
            fputs(" Request Timed Out\n", f);
            break;
        case HTTP_SERVER_ERROR:
            fputs(" Internal Server Error\n", f);
            break;
        case HTTP_NOT_IMPLEMENTED:
            fputs(" Method Not Implemented\n", f);
            break;
        case HTTP_VERSION_NOT_SUPPORTED:
            fputs(" Version Not Supported\n", f);
            break;
        default:
            fputs("\n", f);

        }
        
        for (i = 0; i < resp->nheaders; i++)
            fprintf(f, "%s: %s\n", resp->headers[i].name,
                resp->headers[i].value);

        fputs("\n", f);

        if (resp->content != NULL && strlen(resp->content) != 0) {
            fputs(resp->content, f);
            fputs("\n", f);
        }

        fputs("\n", f);

        /* Flush the output stream */
        fflush(f);

        return true;
    }

    return false;
}
