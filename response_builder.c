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
#include <limits.h>
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

#define MAX_LENGTH 128

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
response_set_file(t_httpresp *resp, char *path, time_t modifiedsince)
{
    struct stat st;
    char *mime;
    magic_t mag;
    int fd;

    if (stat(path, &st) == -1) {

        switch (errno) {
        case ENOENT:
            /* File not found */
            resp->status = HTTP_NOT_FOUND;
            if (strcmp(path, FILE_404_PAGE) != 0)
                return response_set_file(resp, FILE_404_PAGE, -1);
            break;

        default:
            /* Other error */
            resp->status = HTTP_SERVER_ERROR;
        }

        return false;
    }

    resp->lastmodified = st.st_mtime;

    if (modifiedsince != -1 && modifiedsince > st.st_mtime) {
        resp->status = HTTP_NOT_MODIFIED;
        return true;
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
        
        makeIndex(path,index);
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
response_set_cgi(t_httpresp *resp, char *path, char *cgi_path, time_t modifiedsince){
    struct stat st;
    int pid, p[2];
    char tpath[MAX_LENGTH];

    if (sprintf(tpath, "%s/%s", cgi_path, &path[8]) < 0)
        return false;

    if (debug)
        printf("Resolved CGI path to %s\n", tpath);

    /** Check existence of the file requested */
    if (access(tpath, F_OK) != 0){
        resp->status = HTTP_NOT_FOUND;
        return false;
    }
        
    if (stat(tpath, &st) == -1){
        resp->status = HTTP_SERVER_ERROR;
        return false; 
    }

    if (S_ISDIR(st.st_mode)){
        /** If DIR then handle it like normal file */
        return response_set_file(resp, tpath, modifiedsince);
    } else {
        if (debug)
            printf("Checking permissions of %s \n", tpath);
        
        /** Check to make sure file has execution permissions */
        if (access(tpath, X_OK) != 0){
            resp->status = HTTP_FORBIDDEN;
            return false;
        } else {            
            //TODO If not a GET- add logic for POST

            if (pipe(p) == -1 || (pid = fork()) < 0){
                resp-> status = HTTP_SERVER_ERROR;
                return false;
            }
            if (pid == 0){
                char temp[MAX_LENGTH] = "";
                char *envir[] = {"PATH=/tmp", temp, NULL};
                close(p[0]);
                
                if (debug) {
                    printf("org FD: %d \n", resp->content_fd);
                    printf("cgi script: %s \n", tpath);
                }

                /* Push output of result to orginal socket descriptor */
                dup2(p[1], STDOUT_FILENO);
                if (execle(tpath, "", (char *) 0, envir) < 0){
                    resp->status = HTTP_SERVER_ERROR;
                    return false;
                }
                exit(0);
            } else {
                close(p[1]);
                resp->content_fd = p[0];
                resp->content_length = INT_MIN;
                printf("Hidey Ho, piping from %d\n", p[0]);
            }
        }
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
        strftime(buf, BUFSZ, "%a, %d %b %Y %T %z", local)) {

        /* Set the 'Date' response header */
        response_set_header(resp, HEADER_DATE, buf);
    }

    if ((local = localtime(&(resp->lastmodified))) != NULL &&
        strftime(buf, BUFSZ, "%a, %d %b %Y %T %z", local)) {
        /* Set the 'Last-Modified' response header */
        response_set_header(resp, HEADER_LAST_MODIFIED, buf);
    }

    if (resp->content_length >= 0) {
        if (sprintf(buf, "%d", resp->content_length) > 0) {
            /* Set the 'Content-Length' response header */
            response_set_header(resp, HEADER_CONTENT_LENGTH, buf);
        }
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

        /* Flush the output stream */
        fflush(f);

        if (resp->content_fd != -1) {

            if (resp->content_length > 0) {
                fputs("\n", f);
            }

            /* Zero out our buffer because it is being reused */
            memset(buf, 0, BUFSZ);

            /* Read from the fd and write to the response */
            while ((nbytes = read(resp->content_fd, buf, BUFSZ)) > 0) {
                fwrite(buf, sizeof(char), nbytes, f);
            }

        } else if (resp->content != NULL) {

            if (resp->content_length > 0) {
                fputs("\n", f);
            }

            /* Write the text content to the response */
            fputs(resp->content, f);
        }

        fputs("\n", f);

        /* Flush the output stream */
        fflush(f);

        return true;
    }

    return false;
}
