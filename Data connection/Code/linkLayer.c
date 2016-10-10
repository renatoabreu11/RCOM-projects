#include "linkLayer.h"

int *writeDataFromEmissor(int *fd, char *buf) {

//Estabilishing connection
	
	char DADOS = 'c';		//only for compilation purposes
    unsigned char I[7] = {FLAG, A, C_I0, A^C_I0, DADOS, A^C_I0, FLAG};
	rrState current = startRR;	

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

    return 0;
}

int *readDataFromEmissor(int *fd, char *buf) {
	unsigned char RR[5] = {FLAG, A, C_RR1, A^C_RR1, FLAG};
	char buf[255];
	int res;
	int dataReceived = 0;

	iState current = startI;

    int i = 0;
    char message[255];

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);     /* returns after 1 char have been input */
      buf[res]=0;               /* so we can printf... */

	  switch(current){
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
	}
}

void *updateNsNr() {
	if(ns == 0) {
		ns = 1;
		nr = 0;
	}
	else {
		ns = 0;
		nr = 1;
	}
}
