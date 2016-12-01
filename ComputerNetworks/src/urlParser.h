#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 

#include <arpa/inet.h>

/*
 * URL storage
 */
struct parsed_url {
    const char *url;                   
    char *scheme;               /* Url Scheme -> https, ftp, mailto, etc. It is mandatory */
    char *host;                 /* mandatory */
    int port;                 /* optional */
    char *path;                 /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
    char *ip;
    char *filename;
};

struct parsed_url * parse_url(const char *);
int hostToIP(struct parsed_url *);
void printParsedUrl(struct parsed_url *);
void freeUrlStruct(struct parsed_url *);