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
LinkLayer* InitLink();