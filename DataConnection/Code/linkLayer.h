#include "libraries.h"

#define MAX_TRIES 3
#define MAX_SIZE 10
#define BAUDRATE B38400

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//Frame info
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define C_I0 0x00
#define C_I1 0x40
#define C_RR0 0x05
#define C_RR1 0x85
#define C_DISK 0x0b

#define ESCAPE 0x7d
#define FLAG 0x7e

typedef struct LinkLayer {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
}LinkLayer;

/**
*/
LinkLayer *InitLink();

/**
*/
void atende();

/**
 * TRANSMITTER ONLY!
 * Sends SET
*/
int writeSET(int fd);

/**
 * RECEIVER ONLY!
 * Sends UA
*/
int writeUA(int fd);

/**
 * RECEIVER ONLY
 * Sends RR
*/
int writeRR(int fd);

/**
 * resWanted can be 0 (for UA response) or 1 (for RR response)
*/
int waitForEmissorResponse(int fd, int resWanted, LinkLayer *link);

/**
*/
int waitForSET(int fd);

/**
*/
int connectTransmitter(int fd, LinkLayer * link);

/**
*/
int connectReceiver(int fd);

/**
 * Calculates the XOR of the data bytes
 */
int calculateBCC2(char *frame, int length);

/**
*/
int writeDataFrame(int fd, char *buffer, int length);

/**
 * Stores byteRead into the frame, thus storing the image
*/
int readDataInformation(char *frame, char byteRead, char *BCC2);

/**
 * State machine to read the I frame
 * frame is the buffer in which the bytes read are stored
*/
char readDataFrame(int fd, char *frame);

/**
*/
void updateNsNr();


int countPatterns(char* frame, int length);

/**
*/
char* byteStuffing(char* frame, int length);

char* byteDestuffing(char* frame, int length);

/**
*/
int disconnectTransmitter(int fd, LinkLayer * link);

/**
*/
int disconnectReceiver(int fd, LinkLayer * link);
