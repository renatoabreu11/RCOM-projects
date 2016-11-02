#include "linkLayer.h"

volatile int STOP=FALSE;
int timer = 1, flag = 1;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop} state;

LinkLayer *linkLayer;
int numFrameI = 0;
int numFrameItransmitted = 0;
int numTimeOuts = 0;
int numREJreceived = 0;

int initLinkLayer(int port, int status, int baudrate, int retries, int timeout) {
	linkLayer = (LinkLayer *) malloc(sizeof(LinkLayer));
	sprintf(linkLayer->port ,"/dev/ttyS%d", port);
	linkLayer->baudRate = getBaud(baudrate);
	linkLayer->timeout = timeout;
	linkLayer->numTransmissions = retries;
	linkLayer->ns = 0;
	linkLayer->numREJ = 0;
	linkLayer->controlI = C_I;
	linkLayer->controlRR = C_RR;
	linkLayer->controlREJ = C_REJ;

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
	linkLayer->newtio.c_cflag = linkLayer->baudRate | CS8 | CLOCAL | CREAD;
	linkLayer->newtio.c_iflag = IGNPAR;
	linkLayer->newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	linkLayer->newtio.c_lflag = 0;

	linkLayer->newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	linkLayer->newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 char received */

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

int llopen(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Connecting Transmitter");
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_SET);
			if(waitForResponse(fd, UA) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error estabilishing connection!");
			return -1;
		}
	} else if(linkLayer->status == 1){
		printf("%s\n", "Connecting Receiver");
		waitForResponse(fd, SET);
		sendSupervision(fd, C_UA);
	}
	return 1;
}

int llclose(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Disconnecting Transmitter");
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_DISC);
			if(waitForResponse(fd, DISC) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		sendSupervision(fd, C_UA);
		sleep(1);
	} else if(linkLayer->status == 1){
		printf("%s\n", "Disconnecting Receiver");
		int nTry = 1;
		int messageReceived = 0;
		while(!messageReceived){
			if(waitForResponse(fd, DISC) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		nTry = 1;
		messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_DISC);
			if(waitForResponse(fd, UA) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
	}

	if ( tcsetattr(fd,TCSANOW,&linkLayer->oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	close(fd);

	printf("Rej transmissions %d\n", linkLayer->numREJ);

	return 1;
}

int llwrite(unsigned char * buffer, int length, int fd){
	int newSize = length + 6;
	unsigned char *frame = createDataFrame(buffer, length);
	int sizeAfterStuff = byteStuffing(&frame, newSize);

	int bytesSent;
	int nTry = 1;
	int messageReceived = 0;
	while(nTry <= linkLayer->numTransmissions && !messageReceived){
		bytesSent = write(fd, frame, sizeAfterStuff);
		numFrameItransmitted++;
		if(bytesSent != sizeAfterStuff){
			printf("%s\n", "Error sending data packet");
			return -1;
		}
		if(waitForResponse(fd, RR) == -1){
			nTry++;
		} else messageReceived = 1;
	}
	if(!messageReceived){
		printf("%s\n", "Error receiving packet confirmation!");
		return -1;
	}
	return 1;
}

int checkForFrameErrors(int fd, unsigned char *buffer, unsigned char *package, int length, int dataSize) {
	//Check if Control is wrong (using Ns)
	if(buffer[2] != linkLayer->controlI) {
		linkLayer->numREJ++;

		if((buffer[2] << 7) != linkLayer->controlI)
			updateNs();

		sendSupervision(fd, linkLayer->controlREJ);
		//printf("*********************************************************************\nError during Control\n");
		return -1;
	}

	//Checks if BCC1 is wrong
	if(buffer[3] != (buffer[1] ^ buffer[2])) {
		linkLayer->numREJ++;
		sendSupervision(fd, linkLayer->controlREJ);
		printf("Error during BCC1\n");
		return -1;
	}

	//Checks if BCC2 is wrong
	unsigned char bcc2Read = buffer[length - 2];
	unsigned char bcc2FromBytes = calculateBCC2(package, dataSize);

	if(bcc2Read != bcc2FromBytes){
		//printf("Different BCC's\n");
		//printf("bcc2Read = %c, bcc2FromBytes = %c\n", bcc2Read, bcc2FromBytes);
		linkLayer->numREJ++;
		sendSupervision(fd, linkLayer->controlREJ);
		printf("Error during BCC2\n");
		return -1;
	}

	return 0;
}

int llread(int fd, unsigned char *package, int numFrame){
	unsigned char * buffer;
	int length, dataSize;

	while(1) {
		buffer = malloc(MAX_FRAME_LENGTH);

		length = readDataFrame(fd, buffer);
		if(length == -1) {
			printf("Error reading frame, trying again...\n");
			continue;
		}

		printf("Frame length with stuffing: %d\n", length);
		length = byteDestuffing(&buffer, length);
		printf("Frame length after destuff: %d\n",length);

		dataSize = length - 6;
		//printf("Data size = %d\n", dataSize);

		memcpy(package, &buffer[4], dataSize);

		//printf("Esta e a frame %d\n", numFrame);
		if(checkForFrameErrors(fd, buffer, package, length, dataSize) == -1) {
			free(buffer);
			continue;
		}

		//Sends RR
		printf("Frame %d enviando RR\n", numFrame);
		sendSupervision(fd, linkLayer->controlRR);
		updateNs();
		free(buffer);
		break;
	}

	return length - 6;
}

<<<<<<< HEAD
=======
int llclose(int fd){
	if ( tcsetattr(fd,TCSANOW,&linkLayer->oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	close(fd);

	//printf("Rej transmissions %d\n", linkLayer->numREJ);

	return 1;
}

int estabilishConnection(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Connecting Transmitter");
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_SET);
			if(waitForResponse(fd, UA) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error estabilishing connection!");
			return -1;
		}
	} else if(linkLayer->status == 1){
		printf("%s\n", "Connecting Receiver");
		waitForResponse(fd, SET);
		sendSupervision(fd, C_UA);
	}
	return 1;
}

int endConnection(int fd){
	STOP = FALSE;
	timer = 1;
	if(linkLayer->status == 0){
		printf("%s\n", "Disconnecting Transmitter");
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_DISC);
			if(waitForResponse(fd, DISC) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		sendSupervision(fd, C_UA);
		sleep(1);
	} else if(linkLayer->status == 1){
		printf("%s\n", "Disconnecting Receiver");
		int messageReceived = 0;
		while(!messageReceived){
			if(waitForResponse(fd, DISC) == -1)
				continue;
			else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		int nTry = 1;
		messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			sendSupervision(fd, C_DISC);
			if(waitForResponse(fd, UA) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
	}

	return 1;
}

>>>>>>> c34c8389959941ff7ddc4f1514b82999c9fbe1c5
int sendSupervision(int fd, unsigned char control){
	unsigned char frame[5];

	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = control;
	frame[3] = A ^ frame[2];
	frame[4] = FLAG;

	int numBytesSent = write(fd, frame, 5);

	if(numBytesSent != 5) {
		printf("Error sending frame!\n");
		return -1;
	}
	return 0;
}

unsigned char calculateBCC2(unsigned char *frame, int length) {
	int i = 1;
	unsigned char BCC2;

	BCC2 = frame[0];

	for(; i < length; i++)
		BCC2 ^= frame[i];

	return BCC2;
}

unsigned char * createDataFrame(unsigned char *buffer, int length) {
	int newLength = length + 6;
	unsigned char *frame = malloc(newLength);

	unsigned char C = linkLayer->controlI;
	unsigned char BCC1 = A ^ C;
	unsigned char BCC2 = calculateBCC2(buffer, length);

	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = C;
	frame[3] = BCC1;
	memcpy(&frame[4], buffer, length);
	frame[4 + length] = BCC2;
	frame[5 + length] = FLAG;

	return frame;
}

int waitForResponse(int fd, unsigned char flagType) {
	unsigned char control;
	timer = 1;
	switch(flagType){
		case UA: printf("Waiting for UA flag...\n"); control = C_UA; break;
		case RR: /*printf("Waiting for RR flag...\n");*/ control = linkLayer->controlRR; break;
		case DISC: printf("Waiting for DISC flag...\n"); control = C_DISC; break;
		case SET: printf("Waiting for SET flag...\n"); control = C_SET; timer = linkLayer->timeout+1; break;
	}
	unsigned char buf[255];

	int res;
	int receivedREJ = 0;
	if(flagType == SET)
		STOP = FALSE;
	else STOP = TRUE;
	unsigned char wrongRej = linkLayer->controlREJ;
	TOOGLE_BIT(wrongRej, 7);

	state current = start;

	while(timer < linkLayer->timeout+1 || !STOP){
		if(flagType != SET){ //set não usa timer
			if(flag){
				alarm(linkLayer->timeout);                 // activa alarme de 3s
				flag=0;
			}
		}
		res = read(fd,buf,1);     /* returns after 1 char have been input */

		if(res == 0)
			continue;
		else if(res == -1)
			return -1;

		if(res > 0)
			buf[res]=0;

		switch(current){
			case start:
				if(buf[0] == FLAG)
					current = flagRCV;
				break;

			case flagRCV:
				if(buf[0] == A)
					current = aRCV;
				else if(buf[0] != FLAG)
					current = start;
				break;

			case aRCV:
				if(buf[0] == linkLayer->controlREJ || buf[0] == wrongRej || buf[0] == control)
					current = cRCV;
				else if(buf[0] != FLAG)
					current = start;
				else
					current = flagRCV;
				break;

			case cRCV:
				if(buf[0] == (A^linkLayer->controlREJ) || buf[0] == (A^wrongRej)){
					current = BCC;
					receivedREJ = 1;
				}
				else if(buf[0] == (A^control))
					current = BCC;
				else if(buf[0] == FLAG)
					current = start;
				else
					current = flagRCV;
				break;

			case BCC:
				if(buf[0] == FLAG)
					current = stop;
				else
					current = start;

			case stop:
				flag = 1;
				switch(flagType){
					case UA: printf("UA flag received!\n"); break;
					case RR:
					if(receivedREJ == 0){
						printf("RR flag received!\n"); break;
					}
					else{
						linkLayer->numREJ++;
						printf("An error ocurred. REJ flag received!\n");
						return -1;
					}
					case DISC: printf("DISC flag received!\n"); break;
					case SET: printf("SET flag received!\n"); break;
				}

				if(!receivedREJ)
					if(flagType == 1){
						//printf("%s\n", "NS Received and updated");
						updateNs();
					}

					return 1;

			default: break;
		}
	}

	return -1;
}

int countPatterns(unsigned char** frame, int length){
	int patterns = 0;
	int i = 1;
	for(; i < length - 1; i++){
		if((*frame)[i] == ESCAPE || (*frame)[i] == FLAG)
			patterns++;
	}
	return patterns;
}

int byteStuffing(unsigned char** frame, int length) {
	int patterns = countPatterns(frame, length);
	int newLength = length + patterns;
	*frame = realloc(*frame, newLength);

	int i = 1;
	for(; i < length - 1; i++){
		if((*frame)[i] == ESCAPE || (*frame)[i] == FLAG){
			memmove(*frame + i + 1, *frame + i, length - i);
			length++;
			(*frame)[i] = ESCAPE;
			(*frame)[i + 1] ^= 0x20;
		}
	}
	return newLength;
}

int byteDestuffing(unsigned char** frame, int length){
	int patterns = countPatterns(frame, length);

	if(patterns == 0)
		return length;

	int newLength = length - patterns;

	int i = 1;
	for(; i < length - 1; i++){
		if((*frame)[i] == ESCAPE){
			memmove(*frame + i, *frame + i + 1, length - i - 1);
			length--;
			(*frame)[i] ^= 0x20;
		}
	}

	*frame = realloc(*frame, newLength);

	return newLength;
}

void atende(){
	flag=1;
	timer++;
	numTimeOuts++;
}

int readDataFrame(int fd, unsigned char *frame) {
	int res;
	int counter = 0;
	unsigned char byteRead;
	state current = start;
	STOP = FALSE;

	while(STOP == FALSE) {
		res = read(fd, &byteRead, 1);

		if(res == -1)
			return -1;

		//If the number of bytes read is 0, there's no need to put them in the buffer
		if(res == 0)
			continue;

		switch(current){
			case start:
			if(byteRead == FLAG){
				frame[counter++] = byteRead;
				current = flagRCV;
			}
			break;

			case flagRCV:
			if(byteRead == A){
				current = aRCV;
				frame[counter++] = byteRead;
			}
			else if(byteRead != FLAG){
				current = start;
				counter = 0;
			}
			break;

			case aRCV:
			if(byteRead == FLAG){
				current = flagRCV;
				counter = 1;
			}else{
				current = cRCV;
				frame[counter++] = byteRead;
			}
			break;

			case cRCV:
			if(byteRead == FLAG){
				current = flagRCV;
				counter = 1;
			}else if(byteRead == (frame[1] ^ frame[2])){
				current = BCC;
				frame[counter++] = byteRead;
			} else{
				current = start;
				counter = 0;
			}
			break;

			case BCC:
			if(byteRead == FLAG){
				frame[counter++] = byteRead;
				current = stop;

				//We check if there's another byte waiting to be read.
				//If there is, that means the transmitter send the same frame again to be read
				char nextByte;
				res = read(fd, &nextByte, 1);

				if(res == 0)
					STOP = TRUE;
				else if(nextByte == A) {
					current = aRCV;
					frame[0] = FLAG;
					frame[1] = A;
					counter = 2;
				}
				break;
			}else{
				frame[counter++] = byteRead;
				break;
			}

			case stop:
					STOP = TRUE;
				break;

			default: break;
		}
	}

	numFrameI++;
	printf("Numero de tramas = %d\n", numFrameI);
	return counter;
}

void updateNs() {
	if(linkLayer->ns == 0){
		linkLayer->ns = 1;
	}
	else
		linkLayer->ns = 0;

	TOOGLE_BIT(linkLayer->controlI, 6);
	TOOGLE_BIT(linkLayer->controlRR, 7);
	TOOGLE_BIT(linkLayer->controlREJ, 7);
}

int getBaud(int baudrate){
	switch(baudrate){
		case 4800: return B4800;
		case 9600: return B9600;
		case 19200: return B19200;
		case 57600: return B57600;
		case 115200: return B115200;
	}
	return B115200;
}

int getNumREJ() {
	return linkLayer->numREJ;
}

int getTotalITransmissions() {
	return numFrameI;
}

int getNumFrameItransmitted() {
	return numFrameItransmitted;
}

int getNumTimeOuts() {
	return numTimeOuts;
}
