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

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//Frame info
#define ESCAPE 0x7d
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define C_I 0x00
#define C_RR 0x85
#define C_DISC 0x0b
#define C_REJ 0x81

#define UA 0
#define RR 1
#define DISC 2
#define SET 3

#define BIT_MASK(bit)             (1 << (bit))
#define TOOGLE_BIT(value,bit)        ((value) ^= BIT_MASK(bit))

typedef struct LinkLayer {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
	int frameLength;
	int status;
	int ns;
	int numREJtransmissions;
	unsigned int controlI;
	unsigned int controlRR;
	unsigned int controlREJ;
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
int initLinkLayer(int port, int baudRate, int packageSize, int retries, int timeout);

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
int llwrite(unsigned char * buffer, int length, int fd);

/**
 * [llread description]
 * @param  buffer [description]
 * @return        [description]
 */
int llread(int fd, unsigned char * package);

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
 * @param serial port descriptor
 * @param type of flag sent by the emitter or the receiver
 * @param link layer object
 * @return 0 if the flag has been read under the number of transmissions defined, -1 otherwise
 */
int waitForResponse(int fd, unsigned char flagType);

/**
 * @param
 * @param
 * @return
 */
int countPatterns(unsigned char** frame, int length);

/**
 * @param
 * @param
 * @return
 */
int byteStuffing(unsigned char** frame, int length);

/**
 * @param
 * @param
 * @return
 */
int byteDestuffing(unsigned char** frame, int length);


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
unsigned char calculateBCC2(unsigned char *frame, int length);

/**
 * @param
 * @param
 * @param
 * @return
 */
unsigned char * createDataFrame(unsigned char *buffer, int length);

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
int readDataFrame(int fd, unsigned char *frame);

/**
 *
 */
int checkForFrameErrors(int fd, unsigned char *buffer, unsigned char *package, int length, int dataSize);

/**
 *
 */
void updateNs();

/**
*/
int getBaud(int baudrate);
