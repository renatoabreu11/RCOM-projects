#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define BUF_MAX 255

volatile int STOP=FALSE;


#include "applicationLayer.h"
#include "linkLayer.h"

int main(int argc, char** argv){
	ApplicationLayer *app = (ApplicationLayer*) malloc(sizeof(ApplicationLayer));
	app = llopen(1, 0);
}