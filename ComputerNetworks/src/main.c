#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include "urlParser.h"

int main(int argc, char *argv[])
{
	struct parsed_url * parsedUrl = parse_url("ftp://username:password@server.com:8000/public_html/test/");
    printParsedUrl(parsedUrl);
    freeUrlStruct(parsedUrl);
}
