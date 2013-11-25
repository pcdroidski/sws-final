#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HTTP_GET   1
#define HTTP_HEAD  2
#define HTTP_POST  3
#define HTTP_OTHER 0
#define MAX_VERSION_LENGTH      6
#define DEFAULT_VERSION         "0.9"
#define MAX_METHOD_LENGTH       4
#define MAX_URI_LENGTH          1024
#define MAX_STATUS_LENGTH       36

typedef int t_methodtype;

typedef struct httpreq_s {
    int valid;                  /* boolean, validity of the request */
    t_methodtype method_ident;  /* http method identifier */
    char *method;               /* http method as a string */
    int methodlen;              /* length of http method string */
    char *url;                  /* url requested */
    int urllen;                 /* length of url requested */
    char *version;              /* http version */
    int versionlen;             /* length of http version */
    int majorversion;           /* HTTP/X.Y => X */
    int minorversion;           /* HTTP/X.Y => Y */
    char *statuscode;           /* The statuscode of the request*/
    int content;                /* boolean for content returned from GET*/
    //TODO headers map
} t_httpreq;

t_methodtype ident_method(char *);
t_httpreq *parse(char *);
