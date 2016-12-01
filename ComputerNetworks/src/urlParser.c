#include "urlParser.h"

/*
 * Prototype declarations
 */
static __inline__ int _is_scheme_char(int);

/*
 * Check whether the character is permitted in scheme string
 */
static __inline__ int _is_scheme_char(int c)
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

struct parsed_url * parse_url(const char *url)
{
    struct parsed_url *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;

    /* Allocate the parsed url storage */
    purl = malloc(sizeof(struct parsed_url));
    if ( NULL == purl ) {
        return NULL;
    }
    purl->url = url;
    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = 0;
    purl->path = NULL;
    purl->username = NULL;
    purl->password = NULL;
    purl->ip = NULL;
    purl->filename = NULL;

    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */

    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if ( NULL == tmpstr ) {
        printf("Scheme : delimiter not found!\n");
        freeUrlStruct(purl);
        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;
    /* Check restrictions */
    for ( i = 0; i < len; i++ ) {
        if ( !_is_scheme_char(curstr[i]) ) {
            /* Invalid format */
            freeUrlStruct(purl);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->scheme ) {
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';
    /* Make the character to lower if it is upper case. */
    for ( i = 0; i < len; i++ ) {
        purl->scheme[i] = tolower(purl->scheme[i]);
    }
    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for ( i = 0; i < 2; i++ ) {
        if ( '/' != *curstr ) {
            freeUrlStruct(purl);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( '@' == *tmpstr ) {
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        } else if ( '/' == *tmpstr ) {
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if ( userpass_flag ) {
        /* Read username */
        while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr ) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->username = malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->username ) {
            freeUrlStruct(purl);
            return NULL;
        }
        (void)strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';
        /* Proceed current pointer */
        curstr = tmpstr;
        if ( ':' == *curstr ) {
            /* Skip ':' */
            curstr++;
            /* Read password */
            tmpstr = curstr;
            while ( '\0' != *tmpstr && '@' != *tmpstr ) {
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = malloc(sizeof(char) * (len + 1));
            if ( NULL == purl->password ) {
                freeUrlStruct(purl);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }
        /* Skip '@' */
        if ( '@' != *curstr ) {
            freeUrlStruct(purl);
            return NULL;
        }
        curstr++;
    }else{
        purl->username = "anonymous";
    }

    if ( '[' == *curstr ) {
        bracket_flag = 1;
    } else {
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( bracket_flag && ']' == *tmpstr ) {
            /* End of IPv6 address. */
            tmpstr++;
            break;
        } else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) ) {
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->host || len <= 0 ) {
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if ( ':' == *curstr ) {
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '/' != *tmpstr ) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        char * port = malloc(sizeof(char) * (len + 1));
        if ( NULL == port ) {
            freeUrlStruct(purl);
            return NULL;
        }
        (void)strncpy(port, curstr, len);
        port[len] = '\0';
        purl->port = atoi(port);
        curstr = tmpstr;
    }else{
        purl->port = 21;
    }

    /* End of the string */
    if ( '\0' == *curstr ) {
        return purl;
    }

    /* Skip '/' */
    if ( '/' != *curstr ) {
        freeUrlStruct(purl);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr ) {
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->path = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->path ) {
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    char * pch;
    pch=strrchr(purl->path,'/');
    purl->filename = malloc(strlen(pch) - 1);
    memmove(purl->filename, pch + 1, strlen(pch) - 1);

    if(hostToIP(purl) == -1){
        freeUrlStruct(purl);
        return NULL;
    }

    return purl;
}

    /*char keys[] = "/";
    char *pch;
    pch = strpbrk (path, keys);
    while (pch != NULL)
    {
        printf ("%c " , *pch);
        pch = strpbrk (pch+1,keys);
    }
*/

int hostToIP(struct parsed_url *purl){
    	struct hostent *h;      

        if ((h=gethostbyname(purl->host)) == NULL) {  
            herror("gethostbyname");
            return -1;
        }

        purl->ip = inet_ntoa(*((struct in_addr *)h->h_addr));
        return 0;
}

void printParsedUrl(struct parsed_url *purl){
    if ( NULL != purl ) {
        if ( NULL != purl->scheme ) {
            printf("Scheme: %s\n", purl->scheme);
        }
        if ( NULL != purl->host ) {
            printf("Host: %s\n", purl->host);
        }
        printf("Port: %d\n", purl->port);
        if ( NULL != purl->path ) {
            printf("Path: %s\n", purl->path);
        }
        if ( NULL != purl->filename ) {
            printf("Filename: %s\n", purl->filename);
        }
        if ( NULL != purl->username ) {
            printf("Username: %s\n", purl->username);
        }
        if ( NULL != purl->password ) {
            printf("Password: %s\n", purl->password);
        }
        if ( NULL != purl->ip ) {
            printf("IP: %s\n", purl->ip);
        }
    }    
}

/*
 * Free memory of parsed url
 */
void freeUrlStruct(struct parsed_url *purl)
{
    if ( NULL != purl ) {
        if ( NULL != purl->scheme ) {
            free(purl->scheme);
        }
        if ( NULL != purl->host ) {
            free(purl->host);
        }
        if ( NULL != purl->path ) {
            free(purl->path);
        }
        if ( NULL != purl->filename ) {
           free(purl->filename);
        }
        free(purl);
    }
}