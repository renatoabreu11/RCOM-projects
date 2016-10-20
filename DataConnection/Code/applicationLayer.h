#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "linkLayer.h"

#define BUF_MAX 255
#define BAUDRATE B38400
#define CONTROL_DATA 0x01
#define CONTROL_START 0x02
#define CONTROL_END 0x03
#define FILE_SIZE 0x00
#define FILE_NAME 0x01
#define BytesPerPacket 100
#define L2 0x00
#define sizeCodified 0x04

typedef struct ApplicationLayer{
	/*Serial port descriptor*/
	int fileDescriptor;
	/*TRANSMITTER | RECEIVER*/
	int status;

	LinkLayer * link;

	char * fileName;
	int nameLength;
}ApplicationLayer;

ApplicationLayer* InitApplication(int port, int status, char * name);

int startConnection(ApplicationLayer *app);

/**
*/
int llopen(ApplicationLayer *app);

/**
*/
int llwrite(char * buffer, int length, ApplicationLayer* app);

/**
*/
int llread(char * buffer, ApplicationLayer* app);

/**
*/
int llclose(ApplicationLayer* app);

/**
*/
int createStartEnd(ApplicationLayer* , int type);

int sendData(ApplicationLayer* app);

int createDataPackage(char * buffer, int length, ApplicationLayer* app);