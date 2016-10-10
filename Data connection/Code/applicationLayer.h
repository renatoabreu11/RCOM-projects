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

volatile int STOP=FALSE;

typedef struct ApplicationLayer{
	/*Serial port descriptor*/
	int fileDescriptor;
	/*TRANSMITTER | RECEIVER*/
	int status;
}ApplicationLayer;

/**
*/
ApplicationLayer* InitApplication();

/**
*/
int llwrite(int fd, char * buffer, int length, ApplicationLayer* app);

/**
*/
int llread(int fd, char * buffer, ApplicationLayer* app);

/**
*/
int llclose(int fd, ApplicationLayer* app);

