#include "linkLayer.h"

//METER O ALARME SO DEPOIS DE SE ENVIAR ALGO
//Falta o alarm(0)

volatile int STOP=FALSE;
int timer = 1, flag = 1;
int ns = 0, nr = 1;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop} transmitterState;
typedef enum {startSET, flagRCVSET, aRCVSET, cRCVSET, BCCSET, stopSET} setState;
typedef enum {startI, flagRCVI, aRCVI, cRCVI, BCC1I, DATAI, BCC2I, stopI} iState;

LinkLayer *linkLayer;

int initLinkLayer(int port, int baudRate, int packageSize, int retries, int timeout) {
	linkLayer = (LinkLayer *) malloc(sizeof(LinkLayer));
	sprintf(linkLayer->port ,"/dev/ttyS%d", port);
	linkLayer->baudRate = BAUDRATE;
	linkLayer->sequenceNumber = 0;
	linkLayer->timeout = 3;
	linkLayer->numTransmissions = retries;
	linkLayer->frameLength = 1024;

	printf("%s\n", "Link Layer initialized\n");
	return 1;
}

int llopen(int status, int port){
	linkLayer->status = status;
	int fd;

	/*
 	 Open serial port device for reading and writing and not as controlling tty
 	 because we don't want to get killed if linenoise sends CTRL-C.
	*/
	fd = open(linkLayer->port, O_RDWR | O_NOCTTY);
	if (fd <0) {
		perror(linkLayer->port);
		return -1;
	}

  	if ( tcgetattr(fd, &linkLayer->oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		return -1;
	}

	bzero(&linkLayer->newtio, sizeof(linkLayer->newtio));
	linkLayer->newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	linkLayer->newtio.c_iflag = IGNPAR;
	linkLayer->newtio.c_oflag = 0;

  	/* set input mode (non-canonical, no echo,...) */
	linkLayer->newtio.c_lflag = 0;

  	linkLayer->newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  	linkLayer->newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */

	/*
  	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  	leitura do(s) próximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&linkLayer->newtio) == -1) {
		perror("tcsetattr");
		return -1;
	}

	(void) signal(SIGALRM, atende);

	printf("Serial port open with descriptor: %d\n", fd);

	return fd;
}

int llwrite(char * buffer, int length, int fd){
	char frame[linkLayer->frameLength];

	return 1;
}

int llread(char * buffer, int fd){
	return 1;
}

int llclose(int fd){
  if ( tcsetattr(fd,TCSANOW,&linkLayer->oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
  return 1;
}

int estabilishConnection(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Connecting Transmitter");
		sendSupervision(fd, C_SET);
		waitForResponse(fd, 0, linkLayer);
	} else if(linkLayer->status == 1){
		printf("%s\n", "Connecting Receiver");
		waitForSET(fd);
		sendSupervision(fd, C_UA);
	}
	return 1;
}

int endConnection(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Disconnecting Transmitter");
		sendSupervision(fd, C_DISC);
		waitForResponse(fd, 2, linkLayer);
		sendSupervision(fd, C_UA);
	} else if(linkLayer->status == 1){
		printf("%s\n", "Disconnecting Receiver");
		waitForResponse(fd, 2, linkLayer);
		sendSupervision(fd, C_DISC);
		waitForResponse(fd, 0, linkLayer);
	}
	return 1;
}

int sendSupervision(int fd, unsigned char control){
	unsigned char frame[5];

	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = control;
	frame[3] = A ^ frame[2];
	frame[4] = FLAG;

	int numBytesSent = write(fd, frame, 5);

	if(numBytesSent != 5) {
		printf("Error sending Control!\n");
		return -1;
	}
	return 0;
}

int writeRR(int fd) {
	unsigned char RR[5];
	RR[0] = FLAG;
	RR[1] = A;
	if(nr == 0) {
		RR[2] = 0x05;
		RR[3] = (A^0x05);
	} else {
		RR[2] = 0x85;
		RR[3] = (A^0x85);
	}
	RR[4] = FLAG;

	int numBytesSent = write(fd, RR, 5);

	if(numBytesSent != 5) {
		printf("Error sending RR\n");
		return -1;
	}

	return 0;
}

int waitForSET(int fd) {
	printf("Waiting for Set flag...\n");
	char buf[255];
	int res;
	int setReceived = 0;
	setState current = startSET;

	while (STOP==FALSE) {       /* loop for input */
		res = read(fd,buf,1);     /* returns after 1 char have been input */
		buf[res]=0;               /* so we can printf... */

		if(setReceived == 0)
		switch(current){
			case startSET:
			if(buf[0] == FLAG)
			current = flagRCVSET;
			break;
			case flagRCVSET:
			if(buf[0] == A)
			current = aRCVSET;
			else if(buf[0] != FLAG)
			current = startSET;
			break;
			case aRCVSET:
			if(buf[0] == C_SET)
			current = cRCVSET;
			else if(buf[0] != FLAG)
			current = startSET;
			else
			current = flagRCVSET;
			break;
			case cRCVSET:
			if(buf[0] == (A^C_SET))
			current = BCCSET;
			else if(buf[0] != FLAG)
			current = startSET;
			else
			current = flagRCVSET;
			break;
			case BCCSET:
			if(buf[0] == FLAG)
			current = stopSET;
			else
			current = startSET;
			case stopSET:
			setReceived = 1;
			STOP = TRUE;
			printf("Set received!\n");
			break;
			default: break;
		}
	}
	return 0;
}

int waitForResponse(int fd, int flagType, LinkLayer *link) {
	switch(flagType){
		case 0: printf("Waiting for UA flag...\n"); break;
		case 1: printf("Waiting for RR flag...\n"); break;
		case 2: printf("Waiting for DISC flag...\n"); break;
	}
	char buf[255];
	int connected = 0;
	int resReceived = 0;
	int res;

	transmitterState current = start;

	int j = 0;
	for(; j <= link->numTransmissions; j++){
		if(connected == 1)
			break;

		while(timer < 4 && connected == 0){
			if(flag){
				alarm(3);                 // activa alarme de 3s
				flag=0;
			}

			res = read(fd,buf,1);     /* returns after 1 char have been input */
			buf[res]=0;

			if(resReceived == 0){
				switch(current){
					case start:{
						if(buf[0] == FLAG)
							current = flagRCV;
						break;
					}

					case flagRCV:{
						if(buf[0] == A)
							current = aRCV;
						else if(buf[0] != FLAG)
							current = start;
						break;
					}

					case aRCV:{
						switch(flagType){
							case 0:{
								if(buf[0] == C_UA)
									current = cRCV;
								else if(buf[0] != FLAG)
									current = start;
								else
									current = flagRCV;
								break;
							}
							case 1:{
								if(nr == 0) {
									if(buf[0] == C_RR0)
										current = cRCV;
									else if(buf[0] != FLAG)
										current = start;
									else
										current = flagRCV;
								} else if(nr == 1) {
									if(buf[0] == C_RR1)
										current = cRCV;
									else if(buf[0] != FLAG)
										current = start;
									else
										current = flagRCV;
								}
								break;
							}
							case 2:{
								if(buf[0] == C_DISC)
									current = cRCV;
								else if(buf[0] != FLAG)
									current = start;
								else
									current = flagRCV;
								break;
							}
							break;
						}
						break;
					}

					case cRCV:{
						switch(flagType){
							case 0:{
								if(buf[0] == (A^C_UA))
									current = BCC;
								else if(buf[0] != FLAG)
									current = start;
								else
									current = flagRCV;
								break;
							}
							case 1:{
								if(nr == 0) {
									if(buf[0] == (A^C_RR0))
										current = BCC;
									else if(buf[0] != FLAG)
										current = start;
									else
										current = flagRCV;
								} else if (nr == 1) {
									if(buf[0] == (A^C_RR1))
										current = BCC;
									else if(buf[0] != FLAG)
										current = start;
									else
										current = flagRCV;
								}
								break;
							}
							case 2:{
								if(buf[0] == (A^C_DISC))
									current = BCC;
								else if(buf[0] != FLAG)
									current = start;
								else
									current = flagRCV;
								break;
							}
							default: break;
						}
						break;
					}

					case BCC:{
						if(buf[0] == FLAG)
							current = stop;
						else
							current = start;
					}

					case stop:{
						connected = 1;
						flag = 1;
						switch(flagType){
							case 0: printf("UA flag received!\n"); break;
							case 1: printf("RR flag received!\n"); break;
							case 2: printf("DISC flag received!\n"); break;
						}
						return 0;
					}
					default: break;
				}
			}
		}
	}
	return -1;
}

int countPatterns(char* frame, int length){
	int patterns = 0;
	int i = 0;
	for(; i < length; i++){
		if(frame[i] == ESCAPE || frame[i] == FLAG)
			patterns++;
	}
	return patterns;
}

char* byteStuffing(char* frame, int length) {
	int patterns = countPatterns(frame, length);
	int newLength = length + patterns;
	char * trama = malloc(newLength);

	int i = 0;
	int counter = 0;
	for(; i < length; i++){
		if(frame[i] == ESCAPE){
			trama[counter] = ESCAPE;
			counter++;
			trama[counter] = ESCAPE ^ 0x5d;
		}else if(frame[i] == FLAG){
			trama[counter] = FLAG;
			counter++;
			trama[counter] = FLAG ^ 0x5e;
		}else{
			trama[counter] = frame[i];
		}
		counter++;
	}
	return trama;
}

char* byteDestuffing(char* frame, int length){
	int patterns = countPatterns(frame, length);
	int newLength = length - patterns;
	char * trama = malloc(newLength);

	int i = 0;
	int counter = 0;
	for(; i < length; i++){
		if(frame[i] == ESCAPE || frame[i] == FLAG){
			trama[counter] = frame[i];
			i++;
		}else{
			trama[counter] = frame[i];
		}
		counter++;
	}
	return trama;
}

//organizar a partir daqui 

void atende(){
	flag=1;
	timer++;
}

int calculateBCC2(char *frame, int length) {
	int i = 0;
	char BCC2;

	for(; i < length; i++)
		BCC2 ^= frame[4 + i];

	frame[4 + length] = BCC2;
	return 1;
}

char * createDataFrame(char *buffer, int length) {
	char *frame = malloc(length + 6);

	frame[0] = FLAG;
	frame[1] = A;
	if(ns == 0)
		frame[2] = C_I0;
	else
		frame[2] = C_I1;
	frame[3] = frame[1] ^ frame[2];
	memcpy(&frame[4], buffer, length);
	calculateBCC2(frame, length);
	frame[5 + length] = FLAG;

	//write(fd, frame, length + 6);
	return 1;
}

int readDataInformation(char *frame, char byteRead, char *BCC2) {
	//if(byteRead == )
}

char readDataFrame(int fd, char *frame) {
	//char *xorBCC;
	int res;
	char byteRead;
	char *BCC2;
	int decide;
	int connected;
	iState current = startI;
	STOP = FALSE;

	while(STOP == FALSE) {
		res = read(fd, &byteRead, 1);

		switch(current){
			case start:
				if(byteRead == FLAG)
					current = flagRCV;
				break;
			case flagRCV:
				if(byteRead == A)
					current = aRCV;
				else if(byteRead != FLAG)
					current = start;
				break;
			case aRCV:
				if(ns == 0) {
					if(byteRead == C_I0)
						current = cRCV;
					else if(byteRead != FLAG)
						current = start;
					else
						current = flagRCV;
				} else if(ns == 1) {
					if(byteRead == C_I1)
						current = cRCV;
					else if(byteRead != FLAG)
						current = start;
					else
						current = flagRCV;
				}
				break;
			case cRCV:
				if(ns == 0) {
					if(byteRead == (A^C_I0))
						current = BCC;
					else if(byteRead != FLAG)
						current = start;
					else
						current = flagRCV;
				} else if(ns == 1) {
					if(byteRead == (A^C_I1))
						current = BCC;
					else if(byteRead != FLAG)
						current = start;
					else
						current = flagRCV;
				}
				break;
			case BCC:
				decide = readDataInformation(frame, byteRead, BCC2);
			case stop:
				connected = 1;
				flag = 1;
				printf("Receiver has received all the data!\n");
				break;
			default: break;
		}
	}

	return frame;
}

void updateNsNr() {
	/*if(ns == 0) {
	ns = 1;
	nr = 0;
}
else {
ns = 0;
nr = 1;
}*/
}