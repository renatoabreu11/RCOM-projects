#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include "urlParser.h"
#include "ftp.h"

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Usage:\t./main <URL>\n\tex: ./main ftp://username:password@server.com:8000/public_html/\n");
		exit(1);
    }

    // Parse the url 
	struct parsed_url * parsedUrl = parse_url(argv[1]);
    if(parsedUrl == NULL){
        printf("Invalid URL. URL format: scheme:[//[user:password@]host[:port]][/]path");
        exit(1);
    }

    if(parsedUrl->password == NULL){
        printf("Anonymous Mode. Enter your password (fe.up.pt personal mail):\n");
        parsedUrl->password = malloc(100);
        fgets(parsedUrl->password, 100, stdin);
    }
    printParsedUrl(parsedUrl);

    struct ftp_data *ftp =  malloc(sizeof(struct ftp_data)); 

    // Estabilish server connection
    if(ftpConnect(ftp, parsedUrl->ip, parsedUrl->port) != 1){
        printf("Error estabilishing server connection!\n");
        exit(1);
    }else printf("Successfull connection to server!\n");

    // Authentication
    if(ftpLogin(ftp, parsedUrl->username, parsedUrl->password) != 1){
        printf("Authentication failure!\n");
        exit(1);
    }else printf("Successfull Authentication to server!\n");

    freeUrlStruct(parsedUrl);

    return 1;
}
