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
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

#include "applicationLayer.h"
#include "linkLayer.h"

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

	ApplicationLayer *app = (ApplicationLayer*) malloc(sizeof(ApplicationLayer));
	llopen(app, port, status);

	return 1;
}
