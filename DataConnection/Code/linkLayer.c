#include "linkLayer.h"

//METER O ALARME SO DEPOIS DE SE ENVIAR ALGO
//Falta o alarm(0)

volatile int STOP=FALSE;
int timer = 1, flag = 1;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop} state;

LinkLayer *linkLayer;

int initLinkLayer(int port, int baudrate, int packageSize, int retries, int timeout) {
	linkLayer = (LinkLayer *) malloc(sizeof(LinkLayer));
	sprintf(linkLayer->port ,"/dev/ttyS%d", port);
	linkLayer->baudRate = getBaud(baudrate);
	linkLayer->sequenceNumber = 0;
	linkLayer->timeout = timeout;
	linkLayer->numTransmissions = retries;
	linkLayer->frameLength = packageSize;
	linkLayer->ns = 0;
	linkLayer->numREJtransmissions = 0;

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
	linkLayer->newtio.c_cflag = linkLayer->baudRate | CS8 | CLOCAL | CREAD;
	linkLayer->newtio.c_iflag = IGNPAR;
	linkLayer->newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	linkLayer->newtio.c_lflag = 0;

	//Valor que da = 2
	linkLayer->newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
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

int llwrite(unsigned char * buffer, int length, int fd){
	printf("Frame Initial length: %d\n", length);
	int newSize = length + 6;
	unsigned char *frame = createDataFrame(buffer, length);
	printf("Frame length after header and trailer: %d\n", newSize);
	int sizeAfterStuff = byteStuffing(&frame, newSize);
	printf("Frame length after stuffing: %d\n", sizeAfterStuff);

	int bytesSent = write(fd, frame, sizeAfterStuff);
	if(bytesSent != sizeAfterStuff){
		printf("%s\n", "Error sending data packet");
		return -1;
	}
	int nTry = 1;
	int messageReceived = 0;
	while(nTry <= linkLayer->numTransmissions && !messageReceived){
		if(waitForResponse(fd, 1) == -1){
			bytesSent = write(fd, frame, sizeAfterStuff);
			if(bytesSent != sizeAfterStuff){
				printf("%s\n", "Error sending data packet");
				return -1;
			}
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
	//Check if Control is wrong
	if(linkLayer->ns == 0) {
		if(buffer[2] != C_I0) {
			linkLayer->numREJtransmissions++;
			sendSupervision(fd, C_REJ_1);
		}
	} else {
		if(buffer[2] != C_I1) {
			linkLayer->numREJtransmissions++;
			sendSupervision(fd, C_REJ_0);
		}

		return -1;
	}

	//Checks if BCC1 is wrong
	if(buffer[3] != (buffer[1] ^ buffer[2])) {
		linkLayer->numREJtransmissions++;
		if(linkLayer->ns == 0)
			sendSupervision(fd, C_REJ_1);
		else
			sendSupervision(fd, C_REJ_0);

		return -1;
	}

	//Checks if BCC2 is wrong
	unsigned char bcc2Read = buffer[length - 2];
	unsigned char bcc2FromBytes = calculateBCC2(package, dataSize);

	if(bcc2Read != bcc2FromBytes){
		printf("Different BCC's\n");
		printf("bcc2Read = %c, bcc2FromBytes = %c\n", bcc2Read, bcc2FromBytes);
		linkLayer->numREJtransmissions++;
		if(linkLayer->ns == 0)
			sendSupervision(fd, C_REJ_1);
		else
			sendSupervision(fd, C_REJ_0);

		return -1;
	}

	return 0;
}

int llread(int fd, unsigned char *package){
	unsigned char * buffer = malloc(150);
	int length = readDataFrame(fd, buffer);
	printf("Frame length with stuffing: %d\n", length);

	length = byteDestuffing(&buffer, length);
	printf("Frame length after destuff: %d\n",length);

	int dataSize = length - 6;
	printf("Data size = %d\n", dataSize);

	memcpy(package, &buffer[4], dataSize);

	if(checkForFrameErrors(fd, buffer, package, length, dataSize) == -1)
		return -1;

	//Sends RR
	if(linkLayer->ns == 0)
		sendSupervision(fd, C_RR1);
	else
		sendSupervision(fd, C_RR0);

	updateNs();
	free(buffer);

	return length - 6;
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
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			if(waitForResponse(fd, 0) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error estabilishing connection!");
			return -1;
		}
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
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			if(waitForResponse(fd, 2) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		sendSupervision(fd, C_UA);
	} else if(linkLayer->status == 1){
		printf("%s\n", "Disconnecting Receiver");
		int nTry = 1;
		int messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			if(waitForResponse(fd, 2) == -1){
				nTry++;
			} else messageReceived = 1;
		}
		if(!messageReceived){
			printf("%s\n", "Error terminating connection!");
			return -1;
		}
		sendSupervision(fd, C_DISC);
		nTry = 1;
		messageReceived = 0;
		while(nTry <= linkLayer->numTransmissions && !messageReceived){
			if(waitForResponse(fd, 0) == -1){
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

unsigned char calculateBCC2(unsigned char *frame, int length) {
	int i = 0;
	unsigned char BCC2;

	for(; i < length; i++)
	BCC2 ^= frame[i];

	return BCC2;
}

unsigned char * createDataFrame(unsigned char *buffer, int length) {
	int newLength = length + 6;
	unsigned char *frame = malloc(newLength);

	char C;
	if(linkLayer->ns == 0)
		C = C_I0;
	else
		C = C_I1;

	char BCC1 = A ^ C;
	char BCC2 = calculateBCC2(buffer, length);

	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = C;
	frame[3] = BCC1;
	memcpy(&frame[4], buffer, length);
	frame[4 + length] = BCC2;
	frame[5 + length] = FLAG;

	return frame;
}

int waitForSET(int fd) {
	printf("Waiting for Set flag...\n");
	char buf[255];
	int res;
	int setReceived = 0;
	state current = start;

	while (STOP==FALSE) {       /* loop for input */
		res = read(fd,buf,1);     /* returns after 1 char have been input */
		buf[res]=0;               /* so we can printf... */

	if(setReceived == 0)
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
			if(buf[0] == C_SET)
				current = cRCV;
			else if(buf[0] != FLAG)
				current = start;
			else
				current = flagRCV;
			break;
			case cRCV:
			if(buf[0] == (A^C_SET))
				current = BCC;
			else if(buf[0] != FLAG)
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
			setReceived = 1;
			STOP = TRUE;
			printf("Set received!\n");
			break;
			default: break;
		}
	}
	return 0;
}

int waitForResponse(int fd, int flagType) {
	switch(flagType){
		case 0: printf("Waiting for UA flag...\n"); break;
		case 1: printf("Waiting for RR flag...\n"); break;
		case 2: printf("Waiting for DISC flag...\n"); break;
	}
	unsigned char buf[255];
	int res;
	timer = 1;
	int receivedREJ = 0;

	state current = start;

	while(timer < linkLayer->timeout+1){
		if(flag){
			alarm(linkLayer->timeout);                 // activa alarme de 3s
			flag=0;
		}
		res = read(fd,buf,1);     /* returns after 1 char have been input */
		if(res > 0)
			buf[res]=0;

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
						if(linkLayer->ns == 1){
							if(buf[0] == C_RR0)
							current = cRCV;
							else if(buf[0] != FLAG)
							current = start;
							else
							current = flagRCV;
						} else if(linkLayer->ns == 0){
							if(buf[0] == C_RR1){
								current = cRCV;
							}
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
						if(linkLayer->ns == 1) {
							if(buf[0] == (A^C_RR0))
							current = BCC;
							else if(buf[0] == (A^C_REJ_0)) {
								current = BCC;
								receivedREJ = 1;
							} else if(buf[0] != FLAG)
							current = start;
							else
							current = flagRCV;
						} else if (linkLayer->ns == 0) {
							if(buf[0] == (A^C_RR1))
							current = BCC;
							else if(buf[0] == (A^C_REJ_1)) {
								current = BCC;
								receivedREJ = 1;
							} else if(buf[0] != FLAG)
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
				flag = 1;
				switch(flagType){
					case 0: printf("UA flag received!\n"); break;
					case 1:
						if(receivedREJ == 0) {
							printf("RR flag received!\n"); break;
						} else {
							printf("An error ocurred. REJ flag received!\n"); break;
						}
					case 2: printf("DISC flag received!\n"); break;
				}
				if(flagType == 1){
					printf("%s\n", "NS Received and updated");
					updateNs();
				}
				return 1;
			}
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
}

int readDataFrame(int fd, unsigned char *frame) {
	int res;
	int counter = 0;
	unsigned char byteRead;
	state current = start;
	STOP = FALSE;
	int aux = 0;

	while(STOP == FALSE) {
		res = read(fd, &byteRead, 1);
		printf("Byte read: %c\n", byteRead);
		if(res == -1)
			return -1;

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
			}else{
				frame[counter++] = byteRead;
				aux++;
				break;
			}
			case stop:{
				STOP = TRUE;
				break;
			}

			default: break;
		}
	}

	printf("Foram guardados %d bytes!\n", aux);
	return counter;
}

void updateNs() {
	if(linkLayer->ns == 0)
		linkLayer->ns = 1;
	else
		linkLayer->ns = 0;
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
