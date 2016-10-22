#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#include "applicationLayer.h"

int main(int argc, char **argv) {
	if ( (argc < 3) ||
	((strcmp("0", argv[1])!=0) &&
	(strcmp("1", argv[1])!=0) ) ||
	((strcmp("0", argv[2])!=0) &&
	(strcmp("1", argv[2])!=0) )) {
		printf("Usage:\t./main Port Status\n\tex: ./main 0 1\n");
		exit(1);
	}

	char *p, *c;
	int port, status;

	errno = 0;
	long conv = strtol(argv[1], &p, 10);

	if (errno != 0 || *p != '\0' || conv > INT_MAX) {
		printf("Invalid port value. It must be a valid int.\n");
	} else {
		port = conv;
	}

	errno = 0;
	conv = strtol(argv[2], &c, 10);
	if (errno != 0 || *c != '\0' || conv > INT_MAX) {
		printf("Invalid status value. It must be a valid int.\n");
	} else {
		status = conv;
	}

	char option;
	int baudRate = 0;
	int packageLength = 0;
	int retries = 0;
	int timeOut = 0;

	printf("1 - Start Connection\n"); 
	printf("2 - Baud rate\n"); 
	printf("3 - Data package length\n"); 
	printf("4 - Retries\n"); 
	printf("5 - Time out\n"); 
	printf("6 - Quit\n");  
	printf("Choose your option:\n");

  	do{ 
  		scanf(" %c",&option);

 		switch(option){
  		case'1': 
  			printf("\n\n");
  			InitApplication(port, status, "../pinguim.gif", baudRate, packageLength, retries, timeOut);
  			break;
  		case'2':
  			break;
  		case'3':
  			break;
  		case'4':
  			break;
  		case'5':
  			break;
  		case'6':
  			exit(0);
  		default:
     	 	printf("invalid input, please type again:");
  		}
  }while(option<'1' ||option>'6');

	return 1;
}
