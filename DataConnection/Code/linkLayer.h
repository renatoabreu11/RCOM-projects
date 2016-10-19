#include "libraries.h"

#define C_I0 0x00
#define C_I1 0x40
#define C_RR0 0x05
#define C_RR1 0x9896e5		//check this one, supposed to be 10000101
#define MAX_TRIES 3
#define MAX_SIZE 10
#define BAUDRATE B38400

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define ESCAPE 0x7d

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
typedef enum {startI, flagRCVI, aRCVI, cRCVI, BCC1I, DADOSI, BCC2I, stopI} iState;

/**
*/
//LinkLayer *initLink();

/**
*/
void atende();

/**
 * TRANSMITTER ONLY!
 * Sends SET
*/
int writeSET(int fd);

/**
 * EMITTER ONLY!
 * Sends UA
*/
int writeUA(int fd);

/**
*/
int waitForUA(int fd);

/**
*/
int connectTransmitter(int fd);

/**
*/
int generateFrame(char *frame);

/**
*/
int generateDataFrame();

/**
*/
int generateControlFrame();

/**
*/
void updateNsNr();

/**
*/
char byteStuffer(char* raw);
