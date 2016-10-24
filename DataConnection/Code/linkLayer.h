#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

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
#define C_DISC 0x0b

#define ESCAPE 0x7d
#define FLAG 0x7e

typedef struct LinkLayer {
	char port[20];
	char baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
	int frameLength;
	int status;
	int ns;
	struct termios oldtio, newtio;
}LinkLayer;


/**
 * [initLinkLayer description]
 * @param  port        [description]
 * @param  baudRate    [description]
 * @param  packageSize [description]
 * @param  retries     [description]
 * @param  timeout     [description]
 * @return             [description]
 */
int initLinkLayer(int port, char *baudRate, int packageSize, int retries, int timeout);

/**
 * [llopen description]
 * @param  status [description]
 * @param  port   [description]
 * @return        [description]
 */
int llopen(int status, int port);

/**
 * [llwrite description]
 * @param  buffer [description]
 * @param  length [description]
 * @param  fd     [description]
 * @return        [description]
 */
int llwrite(char * buffer, int length, int fd);

/**
 * [llread description]
 * @param  buffer [description]
 * @return        [description]
 */
char *llread(int fd);

/**
 * [llclose description]
 * @param  fd [description]
 * @return    [description]
 */
int llclose(int fd);

/**
 * [estabilishConnection description]
 * @param  fd [description]
 * @return    [description]
 */
int estabilishConnection(int fd);

/**
 * [endConnection description]
 * @param  fd [description]
 * @return    [description]
 */
int endConnection(int fd);

/**
 * [sendSupervision description]
 * @param  fd      [description]
 * @param  control [description]
 * @return         [description]
 */
int sendSupervision(int fd, unsigned char control);

/**
 * Writes, to file with descriptor fd, RR flag
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int writeRR(int fd);

/**
 * Reads from serial port until it finds the SET flag
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int waitForSET(int fd);

/**
 * @param serial port descriptor
 * @param type of flag sent by the emitter or the receiver
 * @param link layer object
 * @return 0 if the flag has been read under the number of transmissions defined, -1 otherwise
 */
int waitForResponse(int fd, int flagType, LinkLayer *link);

/**
 * @param
 * @param
 * @return
 */
int countPatterns(char** frame, int length);

/**
 * @param
 * @param
 * @return
 */
int byteStuffing(char** frame, int length);

/**
 * @param
 * @param
 * @return
 */
int byteDestuffing(char** frame, int length);


//comentar e organizar a partir daqui


/**
 *
 */
void atende();

/**
 * Calculates the XOR of the data bytes
 * @param
 * @param
 * @return
 */
int calculateBCC2(char *frame, int length);

/**
 * @param
 * @param
 * @param
 * @return
 */
char * createDataFrame(char *buffer, int length);

/**
 * Stores byteRead into the frame, thus storing the image
 * @param
 * @param
 * @param
 * @return
 */
int readDataInformation(char *frame, char byteRead);

/**
 * State machine to read the I frame
 * frame is the buffer in which the bytes read are stored
*/
/**
 * State machine to read the I frame
 * frame is the buffer in which the bytes read are stored
 * @param
 * @param
 * @return
 */
int readDataFrame(int fd, char *frame);

/**
 *
 */
void updateNs();
