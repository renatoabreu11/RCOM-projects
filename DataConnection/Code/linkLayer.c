#include "linkLayer.h"

//METER O ALARME SO DEPOIS DE SE ENVIAR ALGO
//Falta o alarm(0)

volatile int STOP=FALSE;
int timer = 1, flag = 1;
int ns = 0, nr = 1;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop} transmitterState;
typedef enum {startSET, flagRCVSET, aRCVSET, cRCVSET, BCCSET, stopSET} setState;
typedef enum {startI, flagRCVI, aRCVI, cRCVI, BCC1I, DATAI, BCC2I, stopI} iState;

LinkLayer *InitLink() {
	LinkLayer *linkLayer = (LinkLayer *) malloc(sizeof(LinkLayer));
	linkLayer->baudRate = BAUDRATE;
	linkLayer->sequenceNumber = 0;
	linkLayer->timeout = 3;

	return linkLayer;
}

void atende(){
	flag=1;
	timer++;
}

int writeSET(int fd) {
	unsigned char SET[5] = {FLAG, A, C_SET, A^C_SET, FLAG};
	int numBytesSent = write(fd, SET, 5);

	if(numBytesSent != 5) {
		printf("Error sending SET!\n");
		return -1;
	}

	return 0;
}

int writeUA(int fd) {
	unsigned char UA[5] = {FLAG, A, C_UA, A^C_UA, FLAG};
	int numBytesSent = write(fd, UA, 5);

	if(numBytesSent != 5) {
		printf("Error sending UA!\n");
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

int waitForEmissorResponse(int fd, int resWanted, LinkLayer *link) {
	//Estabilishing connection
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

			if(resReceived == 0)
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
					if(resWanted == 0) {
						if(buf[0] == C_UA)
							current = cRCV;
						else if(buf[0] != FLAG)
							current = start;
						else
							current = flagRCV;
					} else if(resWanted == 1) {
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
					}
					break;
				case cRCV:
					if(resWanted == 0) {
						if(buf[0] == (A^C_UA))
							current = BCC;
						else if(buf[0] != FLAG)
							current = start;
						else
							current = flagRCV;
					} else if(resWanted == 1) {
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
					}
					break;
				case BCC:
					if(buf[0] == FLAG)
						current = stop;
					else
						current = start;
				case stop:
					connected = 1;
					flag = 1;
					printf("Transmitter has received the answer from the emiiter!\n");
					break;
				default: break;
			}
		}
	}
	return 0;
}

int waitForSET(int fd) {
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
			printf("Recebeu SET!\n");
			break;
			default: break;
		}
	}
	return 0;
}

int connectTransmitter(int fd, LinkLayer * link) {
	(void) signal(SIGALRM, atende);
	writeSET(fd);
	waitForEmissorResponse(fd, 0, link);
	return 0;
}

int connectReceiver(int fd) {
	waitForSET(fd);
	writeUA(fd);
	return 0;
}

int calculateBCC2(char *frame, int length) {
	int i = 0;
	char BCC2;

	for(; i < length; i++)
		BCC2 ^= frame[4 + i];

	frame[4 + length] = BCC2;
	return 1;
}

int writeDataFrame(int fd, char *buffer, int length) {
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

	write(fd, frame, length + 6);
	return 1;
}

char *readDataFrame(int fd, char *frame) {
	//char *xorBCC;
	char byteRead;
	iState current = startI;
	STOP = FALSE;

	while(STOP == FALSE) {
		read(fd, &byteRead, 1);

		switch(current) {
			case startI:
				if(byteRead == FLAG)
					current =flagRCVI;
				break;
			case flagRCVI:
				if(byteRead == A)
					current = aRCVI;
				break;
			case aRCVI:
				if(byteRead == (ns << 6))
					current = cRCVI;
				break;
			case cRCVI:
				if(byteRead == (A^(ns << 6)))
					current = BCC1I;
				break;
			case BCC1I:
				current = DATAI;
				break;
			case DATAI:
				break;
			case BCC2I:
				//Fazer o ^ de todos os bytes de DADOSI
				break;
			case stopI:
				//enviar o RR
				writeRR(fd);
				STOP = TRUE;
				break;
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
