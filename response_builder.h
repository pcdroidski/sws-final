#ifndef __RESPONSE_BUILDER_H__
#define __RESPONSE_BUILDER_H__

/*
 * sws - response builder
 *
 * Outlines the types and functions available
 * for managing HTTP response objects, including
 * their header data, content body, etc.
 *
 * Additionally, defines the HTTP status codes
 * that are supported by sws.
 *
 * Author: Nick Smith
 */

#define HTTP_OK                         200
#define HTTP_CREATED                    201
#define HTTP_NOT_MODIFIED               304
#define HTTP_BAD_REQUEST                400
#define HTTP_FORBIDDEN                  403
#define HTTP_NOT_FOUND                  404
#define HTTP_METHOD_NOT_ALLOWED         405
#define HTTP_TIMEOUT                    408
#define HTTP_SERVER_ERROR               500
#define HTTP_NOT_IMPLEMENTED            501
#define HTTP_VERSION_NOT_SUPPORTED      505

#define HEADER_CONTENT_LENGTH           "Content-Length"
#define HEADER_CONTENT_TYPE             "Content-Type"
#define HEADER_DATE                     "Date"
#define HEADER_SERVER                   "Server"
#define HEADER_LAST_MODIFIED            "Last-Modified"

#ifndef bool
#define bool unsigned char
#define false 0
#define true  1
#endif

extern bool debug;
extern int flags_c;

/* Stores header information */
typedef struct httpresponse_header_s {
    char *name;
    char *value;
} t_httpresp_header;

/* Stores a full HTTP response, including headers */
typedef struct httpresponse_s {
    int status;
    char *protocol;
    int major_version;
    int minor_version;
    t_httpresp_header *headers;
    int nheaders, headers_len;
    char *content, *content_type;
    int content_fd, content_length;
    time_t lastmodified;
} t_httpresp;

/* Initialize a response with sensible defaults */
t_httpresp *
init_response();

/* Add a header to the HTTP response */
bool
response_set_header(t_httpresp *, char *name, char *value);

/* Setup the response content to serve a file */
bool
response_set_file(t_httpresp *, char *path, time_t modifiedsince);

/* Setup the response content to serve some text */
bool
response_set_text(t_httpresp *, char *path);

/* Setup the cgi response */
bool
response_set_cgi(t_httpresp *, char *path, char *cgi_path, time_t modifiedsince);

bool
finalize_cgi(t_httpresp *resp, char *path, time_t modifiedsince);

/* Prepare the response to be sent-
 * this should be called when ALL the necessary
 * headers and the content body have been added.
 * This will take care of adding fields like
 * timestamp, content-length, etc.
 */
bool
finalize_response(t_httpresp *);

/* Writes the response to the given file descriptor */
bool
write_response(t_httpresp *, int fd);

/* Destroy a response and free any allocated data */
void
free_response(t_httpresp *);

#endif
