#include "linkLayer.h"

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

int waitForUA(int fd) {
	//Estabilishing connection
	char buf[255];
	int connected = 0;
	int uaReceived = 0;
	int res;

	uaState current = startUA;
	struct termios oldtio;

	int j = 0;
	for(j; j <= MAX_TRIES; j++){
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

	//Can start sending the data here

	return 0;
}

int connectReceiver(int fd) {
	waitForSET(fd);

	//Can start handling receiving bytes here

	return 0;
}

int writeDataFromEmissor(int *fd, char *buf) {

	//Estabilishing connection

	/*int numeroTrama = 0;
	int numBytesRead = 0;
	int numMaxPerData = 100;
	rrState current = startRR;

	//Open file
	FILE *file;
	file = fopen("pinguim.gif", "r");

	char image[numMaxPerData];
	char byteRead;
	int i = 0;


	//Fazer um ciclo de envio
	//Vamos supor que o numero de octetos é 100
	//L2 = 0x00
	//L1 = 0x64		//100 bytes

	//Fill the information to send
	i = 0;
	numBytesRead = 0;
	while((byteRead = getc(file)) != EOF && numBytesRead < numMaxPerData) {
	image[i] = byteRead;
	i++;
	numBytesRead++;
}

//Construct the Trama I
//First the control one
unsigned char[4] controlPackageStart = {CONTROL_FIELD_START, FILE_SIZE, 100, 1};

char numTrama;
sprintf(numTrama, "%d", numeroTrama);
unsigned char data[5] = {CONTROL_FIELD_DATA, numTrama, 0x00, 0x64, image};
unsigned char[4] controlPackageEnd = {CONTROL_FIELD_END, FILE_SIZE, 100, 1};

//Put the 3 tramas all together

//And join them with the Trama I
unsigned char I[7] = {FLAG, A, C_I0, A^C_I0, data, A^C_I0, FLAG};

//Send Trama I though writing
//write(fd, I, );

int rrReceived = 0;
int firstTry = 1;

int i = 0;
for(i; i <= MAX_TRIES; i++){
switch(current){
case startRR: if(buf[0] == FLAG)
current = flagRCVRR;
break;
case flagRCVRR:
if(buf[0] == A)
current = aRCVRR;
else if(buf[0] != FLAG)
current = startRR;
break;
case aRCVRR:	if(buf[0] == C_RR0 || buf[0] == C_RR1)
current = cRCVRR;
else if(buf[0] != FLAG)
current = startRR;
else
current = flagRCVRR;
break;
case cRCVRR:		if(buf[0] == (A^C_RR0) || buf[0] == (A^C_RR1))
current = BCCRR;
else if(buf[0] != FLAG)
current = startRR;
else
current = flagRCVRR;
break;
case BCCRR:	if(buf[0] == FLAG)
current = stopRR;
else
current = startRR;
case stopRR:
printf("Recebeu o RR!\n");
updateNsNr();
break;
default: break;
}
}
*/
return 0;
}

int readDataFromEmissor(int *fd, char *buf) {
	/*unsigned char RR[5] = {FLAG, A, C_RR1, A^C_RR1, FLAG};
	char buf[255];
	int res;
	int dataReceived = 0;

	iState current = startI;

	int i = 0;
	char message[255];

	while (STOP==FALSE) {       /* loop for input */
	/*res = read(fd,buf,1);     /* returns after 1 char have been input */
	/*buf[res]=0;               /* so we can printf... */

	/*switch(current){
	case startI: if(buf[0] == FLAG)
	current = flagRCVI;
	break;
	case flagRCVI:
	if(buf[0] == A)
	current = aRCVI;
	else if(buf[0] != FLAG)
	current = startI;
	break;
	case aRCVI:	if(buf[0] == C_I0 || buf[0] == C_I1)
	current = cRCVI;
	else if(buf[0] != FLAG)
	current = startI;
	else
	current = flagRCVI;
	break;
	case cRCVI:		if(buf[0] == (A^C_I0) || buf[0] == (A^C_I1))
	current = BCC1I;
	else if(buf[0] != FLAG)
	current = startI;
	else
	current = flagRCVI;
	break;
	case BCC1I:	if(buf[0] == FLAG)
	current = DADOSI;
	else
	current = startI;
	break;
	case DADOSI:	if(buf[0] == A^C_I0)
	current = BCC2I;
	else {
	//LER OS DADOS DO TRANSMISSOR
}
break;
case BCC2I: if(buf[0] == FLAG)
current = stopI;
break;
case stopI:	dataReceived = 1;
printf("Recebeu todos os dados!\n");
write(fd, RR, 5);
break;
default: break;
}
}*/

return 1;
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

char byteStuffer(char* raw) {
	/*int j = 0;
	char stuffed[1000]; //Needs to be variable
	for(int i = 0; i < ; raw[i] != NULL){
	if(raw[i]==FLAG){
	stuffed[j++] = ESCAPE;
	stuffed[j++] = FLAG ^ 0x20;
}else if(raw[i]==ESCAPE){
stuffed[j++] = ESCAPE;
stuffed[j++] = ESCAPE ^ 0x20;
}else{
stuffed[j++] = raw[i];
}
}
return stuffed;*/

return 'a';
}
