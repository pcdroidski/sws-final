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

#include <errno.h>
#include <fcntl.h>
#include <magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "response_builder.h"
#include "dir_index.h"

#define BUFSZ       4096
#define SERVER_NAME     "sws"
#define SERVER_VERSION  "0.1"

#define FILE_404_PAGE   "404.html"

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
    resp->status = HTTP_OK;
    resp->headers = NULL;
    resp->nheaders = 0;
    resp->headers_len = 0;
    resp->protocol = strdup("HTTP");
    resp->major_version = 1;
    resp->minor_version = 0;
    resp->content = NULL;
    resp->content_fd = -1;
    resp->content_length = 0;
    resp->content_type = NULL;

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
response_set_file(t_httpresp *resp, char *path)
{
    struct stat st;
    const char *mime;
    magic_t mag;
    int fd;
    
    if (stat(path, &st) == -1) {

        switch (errno) {
        case ENOENT:
            /* File not found */
            resp->status = HTTP_NOT_FOUND;
            if (strcmp(path, FILE_404_PAGE) != 0)
                return response_set_file(resp, FILE_404_PAGE);
            break;

        default:
            /* Other error */
            resp->status = HTTP_SERVER_ERROR;
        }

        return false;
    }

    /* Get the file's MIME type */
    mag = magic_open(MAGIC_MIME_TYPE |
                     MAGIC_NO_CHECK_COMPRESS | MAGIC_NO_CHECK_TAR |
                     MAGIC_NO_CHECK_TOKENS | MAGIC_NO_CHECK_TROFF);
    magic_load(mag, NULL);
    mime = strdup(magic_file(mag, path));
    magic_close(mag);

    /* Open the file for reading */
    if ((fd = open(path, O_RDONLY)) == -1) {
        /* Failed to open the file */
        resp->status = HTTP_SERVER_ERROR;
        return false;
    }
    /* Setup the response content */
    if (S_ISDIR(st.st_mode)){
        char *index;
        index=malloc(MAX_DIR_PAGE + 1);
        
        makeIndex(&path,index);
        response_set_text(resp,index);
    }else{
        resp->content_fd = fd;
        resp->content_length = st.st_size;
        resp->content_type = mime;
    }

    return true;
}

bool
response_set_text(t_httpresp *resp, char *text)
{
    if (resp == NULL) {
        return false;
    }
    
    /* Set the content and content-length */
    if ((resp->content = text) != NULL) {
        resp->content_length = strlen(resp->content);
    }
    
    return true;
}

bool
finalize_response(t_httpresp *resp)
{
    time_t t;
    struct tm *local;

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

    if (sprintf(buf, "%d", resp->content_length) > 0) {
        /* Set the 'Content-Length' response header */
        response_set_header(resp, HEADER_CONTENT_LENGTH, buf);
    }

    /* Set the 'Content-Type' response header */
    response_set_header(resp, HEADER_CONTENT_TYPE,
            (resp->content_type == NULL ?
                "text/html" : resp->content_type));

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
    if (resp->content_type != NULL)
        free(resp->content_type);
    if (resp->protocol != NULL)
        free(resp->protocol);

    /* Close the file descriptor if it's open */
    if (resp->content_fd != -1)
        (void) close(resp->content_fd);

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
    int i, nbytes;

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

        if (resp->content_length > 0) {

            /* Flush the output stream */
            fflush(f);

            if (resp->content_fd != -1) {
                
                /* Zero out our buffer because it is being reused */
                memset(buf, 0, BUFSZ); 

                /* Read from the fd and write to the response */
                while ((nbytes = read(resp->content_fd, buf, BUFSZ)) > 0) {
                    fwrite(buf, sizeof(char), nbytes, f);
                }

            } else if (resp->content != NULL) {

                /* Write the text content to the response */
                fputs(resp->content, f);
            }

            fputs("\n", f);
        }

        fputs("\n", f);

        /* Flush the output stream */
        fflush(f);

        return true;
    }

    return false;
}
