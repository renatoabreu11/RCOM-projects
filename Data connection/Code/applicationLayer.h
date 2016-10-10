#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define BUF_MAX 255

typedef struct ApplicationLayer{
	/*Serial port descriptor*/
	int fileDescriptor;
	/*TRANSMITTER | RECEIVER*/
	int status;
}ApplicationLayer;

/**
*/
ApplicationLayer* llopen(int port, int status);

/**
*/
int llwrite(char * buffer, int length, ApplicationLayer* app);

/**
*/
int llread(char * buffer, ApplicationLayer* app);

/**
*/
int llclose(ApplicationLayer* app);

