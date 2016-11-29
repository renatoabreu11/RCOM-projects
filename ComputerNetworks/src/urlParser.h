#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * URL storage
 */
struct parsed_url {
    const char *url;                   
    char *scheme;               /* Url Scheme -> https, ftp, mailto, etc. It is mandatory */
    char *host;                 /* mandatory */
    char *port;                 /* optional */
    char *path;                 /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
};

struct parsed_url * parse_url(const char *);
void printParsedUrl(struct parsed_url *);
void freeUrlStruct(struct parsed_url *);