#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include "urlParser.h"

int main(int argc, char *argv[])
{
	struct parsed_url * parsedUrl = parse_url("http://65.173.211.241");
    printParsedUrl(parsedUrl);
    freeUrlStruct(parsedUrl);
}
