#include "linkLayer.h"

//METER O ALARME SO DEPOIS DE SE ENVIAR ALGO

volatile int STOP=FALSE;
int timer = 1, flag = 1;
int ns = 0, nr = 1;

typedef enum {startUA, flagRCVUA, aRCVUA, cRCVUA, BCCUA, stopUA} uaState;
typedef enum {startSET, flagRCVSET, aRCVSET, cRCVSET, BCCSET, stopSET} setState;
typedef enum {startRR, flagRCVRR, aRCVRR, cRCVRR, BCCRR, stopRR} rrState;
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

int waitForUA(int fd) {
	//Estabilishing connection
	char buf[255];
	int connected = 0;
	int uaReceived = 0;
	int res;

	uaState current = startUA;
	struct termios oldtio;

	int j = 0;
	for(; j <= MAX_TRIES; j++){
		if(connected == 1)
		break;

		while(timer < 4 && connected == 0){
			if(flag){
				alarm(3);                 // activa alarme de 3s
				flag=0;
			}

			res = read(fd,buf,1);     /* returns after 1 char have been input */
			buf[res]=0;

			if(uaReceived == 0)
			switch(current){
				case startUA:
				if(buf[0] == FLAG)
				current = flagRCVUA;
				break;
				case flagRCVUA:
				if(buf[0] == A)
				current = aRCVUA;
				else if(buf[0] != FLAG)
				current = startUA;
				break;
				case aRCVUA:
				if(buf[0] == C_UA)
				current = cRCVUA;
				else if(buf[0] != FLAG)
				current = startUA;
				else
				current = flagRCVUA;
				break;
				case cRCVUA:
				if(buf[0] == (A^C_UA))
				current = BCCUA;
				else if(buf[0] != FLAG)
				current = startUA;
				else
				current = flagRCVUA;
				break;
				case BCCUA:
				if(buf[0] == FLAG)
				current = stopUA;
				else
				current = startUA;
				case stopUA:
				connected = 1;
				flag = 1;
				printf("Recebeu UA!\n");
				break;
				default: break;
			}
		}
	}

	//Data packets dispatch

	size_t len;
	if(fgets(buf, sizeof(buf), stdin) != NULL){
		len = strlen(buf);
		printf("input: %s", buf);
		printf("string length: %zu\n", len);
	}

	write(fd, buf, len);

	sleep(2);

	char message[255];
	read(fd, message, 255);
	printf("String received: %s\n", message);

	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);

	return 0;
}

int waitForSET(int fd) {
	struct termios oldtio;
	char buf[255];
	int res;
	int setReceived = 0;
	setState current = startSET;

	int i = 0;
	char message[255];

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
			printf("Recebeu SET!\n");
			writeUA(fd);
			break;
			default: break;
		}
		else {
			if (buf[0]=='\n') STOP=TRUE;
			else{
				printf("%s:%d\n", buf, res);
				message[i] = buf[0];
				i++;
			}
		}
	}
	printf("%s\n", message);

	sleep(2);

	write(fd, message, 255);

	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
	return 0;
}

int connectTransmitter(int fd) {
	(void) signal(SIGALRM, atende);
	writeSET(fd);
	waitForUA(fd);

	return 0;
}

int connectReceiver(int fd) {
	waitForSET(fd);

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
	//Esperar pelo RR

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
