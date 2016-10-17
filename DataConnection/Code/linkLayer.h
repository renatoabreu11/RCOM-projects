#include "libraries.h"

#define C_I0 0x00
#define C_I1 0x40
#define C_RR0 0x05
#define C_RR1 0x9896e5		//check this one, supposed to be 10000101
#define MAX_TRIES 3
#define MAX_SIZE 10

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

//Control Package
#define CONTROL_FIELD_START 0X02
#define CONTROL_FIELD_END 0X03
#define FILE_SIZE 0
#define FILE_NAME 1

typedef struct LinkLayer {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
}LinkLayer;

typedef enum {startRR, flagRCVRR, aRCVRR, cRCVRR, BCCRR, stopRR} rrState;
typedef enum {startI, flagRCVI, aRCVI, cRCVI, BCC1I, DADOSI, BCC2I, stopI} iState;

volatile int STOP=FALSE;
int ns = 0, nr = 1;

/**
*/
LinkLayer* InitLink();

/**
*/
int *writeDataFromEmissor(int *fd, char *buf);

/**
*/
int *readDataFromEmissor(int *fd, char *buf);

/**
*/
void *updateNsNr();
