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
#define C_DISC 0x0b

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
 * Initializes the LinkLayer object
 * @return a pointer to a LinkLayer "object"
 */
LinkLayer *InitLink();

/**
 * Estabilishes connection between emitter and receiver
 * @param serial port descriptor
 * @param linkLayer struct 
 * @return 0 if sucessful, -1 otherwise
 */
int connectTransmitter(int fd, LinkLayer * link);

/**
 * Estabilishes connection between emitter and receiver
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int connectReceiver(int fd);

/**
 * Disconnects the connection between emitter and receiver
 * @params erial port descriptor
 * @param linkLayer struct 
 * @return 0 if sucessful, -1 otherwise
 */
int disconnectTransmitter(int fd, LinkLayer * link);

/**
 * Disconnects connection between emitter and receiver
 * @param serial port descriptor
 * @param linkLayer struct 
 * @return 0 if sucessful, -1 otherwise
 */
int disconnectReceiver(int fd, LinkLayer * link);

/**
 * Writes, to file with descriptor fd, SET flag
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int writeSET(int fd);

/**
 * Writes, to file with descriptor fd, UA flag
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int writeUA(int fd);

/**
 * Writes, to file with descriptor fd, DISK flag
 * @param serial port descriptor
 * @return 0 if sucessful, -1 otherwise
 */
int writeDISC(int fd);

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
int writeDataFrame(int fd, char *buffer, int length);

/**
 * Stores byteRead into the frame, thus storing the image
 * @param
 * @param
 * @param
 * @return
 */
int readDataInformation(char *frame, char byteRead, char *BCC2);

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
char readDataFrame(int fd, char *frame);

/**
 * 
 */
void updateNsNr();

/**
 * @param
 * @param
 * @return
 */
int countPatterns(char* frame, int length);

/**
 * @param
 * @param
 * @return
 */
char* byteStuffing(char* frame, int length);

/**
 * @param
 * @param
 * @return
 */
char* byteDestuffing(char* frame, int length);
