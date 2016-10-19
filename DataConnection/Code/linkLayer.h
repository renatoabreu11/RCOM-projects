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

volatile int STOP=FALSE;
int timer = 1, flag = 1;
int ns = 0, nr = 1;		//ISTO E NO LINK LAYER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

typedef enum {startUA, flagRCVUA, aRCVUA, cRCVUA, BCCUA, stopUA} uaState;
typedef enum {startSET, flagRCVSET, aRCVSET, cRCVSET, BCCSET, stopSET} setState;
typedef enum {startRR, flagRCVRR, aRCVRR, cRCVRR, BCCRR, stopRR} rrState;
typedef enum {startI, flagRCVI, aRCVI, cRCVI, BCC1I, DATAI, BCC2I, stopI} iState;

/**
*/
LinkLayer *initLink();

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
*/
int waitForUA(int fd);

/**
*/
int connectTransmitter(int fd);

/**
*/
int writeDataFrame(int fd, char *frame, int size);

/**
 * Reads and stores the actual image data into frame
*/
int readImageData(char *frame, char byteRead, char *xorBCC, iState state);

/**
 * State machine to read the I frame
 * frame is the buffer in which the bytes read are stored
*/
char *readDataFrame(int fd, char *frame);

/**
*/
void updateNsNr();


int countPatterns(char* frame, int length);

/**
*/
char* byteStuffing(char* frame, int length);

char* byteDestuffing(char* frame, int length);
