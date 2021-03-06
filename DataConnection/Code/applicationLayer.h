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
#define CONTROL_DATA 0x01
#define CONTROL_START 0x02
#define CONTROL_END 0x03
#define FILE_SIZE 0x00
#define FILE_NAME 0x01
#define DataHeaders 4
#define L2 0x00
#define RECEIVER 1
#define TRANSMITTER 0

typedef struct ApplicationLayer{
	/*Serial port descriptor*/
	int fileDescriptor;
	/*TRANSMITTER | RECEIVER*/
	int status;

	char * fileName;
	unsigned int fileSize;
	int nameLength;
	int dataLength;
}ApplicationLayer;

/**
 * [InitApplication description]
 * @param  port        [description]
 * @param  status      [description]
 * @param  name        [description]
 * @param  baudRate    [description]
 * @param  packageSize [description]
 * @param  retries     [description]
 * @param  timeout     [description]
 * @return             [description]
 */
int InitApplication(int fileDescriptor, int status, char * name, int packageSize);

/**
 * [startConnection description]
 * @return     [description]
 */
int startConnection();

/**
 * [sendData description]
 * @return [description]
 */
int sendData();

/**
 * [sendControl description]
 * @param  type [description]
 * @return      [description]
 */
int sendControl(int type);

/**
 * [sendInformation description]
 * @param  buffer [description]
 * @param  length [description]
 * @return        [description]
 */
int sendInformation(unsigned char * buffer, int length);

/**
 * [receiveData description]
 * @return [description]
 */
int receiveData();

/**
 * [receiveControl description]
 * @param  type [description]
 * @return      [description]
 */
int receiveControl(int type);

/**
 * [receiveInformation description]
 * @param  buffer [description]
 * @param  length [description]
 * @return        [description]
 */
int receiveInformation(unsigned char *buffer, int *length);

int showReceiverStatistics(int numREJtransmissions, int numTotalITransmissions);

int showTransmitterStatistics(int numREJreceived, int numFrameItransmitted, int numTimeOuts);
