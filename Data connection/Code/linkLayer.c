#include "linkLayer.h"

int writeDataFromEmissor(int fd) {

//Estabilishing connection

    unsigned char I[7] = {FLAG, A, C0, A^C0, DADOS, A^C0, FLAG};
	rrState current = start;	

	//write(fd, I, );

	int ns = 0;
	int rrReceived = 0;
	int firstTry = 1;

	int i = 0;			
	for(i; i <= MAX_TRIES; i++){
		switch(current){
		case start: if(buf[0] == FLAG)
					current = flagRCV;
					break;
		case flagRCV:
					if(buf[0] == A)
						current = aRCV;
					else if(buf[0] != FLAG)
						current = start;
			break;
		case aRCV:	if(buf[0] == C_RR0 || buf[0] == C_RR1)
						current = cRCV;
					else if(buf[0] != FLAG)
						current = start;
					else 
						current = flagRCV;
					break;
		case cRCV:		if(buf[0] == A^C_RR0 || buf[0] == A^C_RR1)
						current = BCC;
					else if(buf[0] != FLAG)
						current = start;
					else
						current = flagRCV;
					break;
		case BCC:	if(buf[0] == FLAG)
						current = stop;
					else
						current = start;
		case stop:	setReceived = 1;
					printf("Recebeu o RR!\n");
					updateNsNr();
					break;
		default: break;
		}	
	}

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}

int readDataFromEmissor(int fd) {
	unsigned char RR[5] = {FLAG, A, RR1, A^RR1, FLAG};
	char buf[255];

	iState current = start;

    int i = 0;
    char message[255];

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);     /* returns after 1 char have been input */
      buf[res]=0;               /* so we can printf... */

	  switch(current){
		case start: if(buf[0] == FLAG)
					current = flagRCV;
					break;
		case flagRCV:
					if(buf[0] == A)
						current = aRCV;
					else if(buf[0] != FLAG)
						current = start;
			break;
		case aRCV:	if(buf[0] == C_I0 || buf[0] == C_I1)
						current = cRCV;
					else if(buf[0] != FLAG)
						current = start;
					else 
						current = flagRCV;
					break;
		case cRCV:		if(buf[0] == A^C_I0 || buf[0] == A^C_I1)
						current = BCC1;
					else if(buf[0] != FLAG)
						current = start;
					else
						current = flagRCV;
					break;
		case BCC1:	if(buf[0] == FLAG)
						current = DADOS;
					else
						current = start;
					break;
		case DADOS:	if(buf[0] == A^CO)
						current = BCC2;
					else {
						//LER OS DADOS DO TRANSMISSOR
					}
					break;
		case BCC2: if(buf[0] == FLAG)
						current = stop;
					break;		
		case stop:	dataReceived = 1;
					printf("Recebeu todos os dados!\n");
					write(fd, RR, 5);
					break;
		default: break;
		}
	}
}

void updateNsNr() {
	if(ns == 0) {
		ns = 1;
		nr = 0;
	}
	else {
		ns = 0;
		nr = 1;
	}
}
