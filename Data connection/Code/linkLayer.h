#define C_I0 0x00
#define C_I1 0x40
#define C_RR0 0x05
#define C_RR1 0x9896e5		//check this one, supposed to be 10000101

typedef struct LinkLayer {
	char port[20];
	int baudRate;
	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int numTransmissions;
	char frame[MAX_SIZE];
}LinkLayer;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop} rrState;
typedef enum {start, flagRCV, aRCV, cRCV, BCC1, DADOS, BCC2, stop} iState;

volatile int STOP=FALSE;
int ns = 0, nr = 1;

/**
*/
LinkLayer* InitLink();

/**
*/
int *writeDataFromEmissor(int fd);

/**
*/
int *readDataFromEmissor(int fd);

/**
*/
void *updateNsNr();
