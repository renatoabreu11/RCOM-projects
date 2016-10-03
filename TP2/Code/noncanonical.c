/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define F 0x7e
#define A 0x03
#define C_SET 0x03
#define C 0x07

volatile int STOP=FALSE;

typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop}setState;

int main(int argc, char** argv)
{
	unsigned char SET[5];
	SET[0] = F;
	SET[1] = A;
 	SET[2] = C_SET;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = F;

	unsigned char UA[5];
	UA[0] = F;
	UA[1] = A;
 	UA[2] = C;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = F;

	unsigned char BCC1 = SET[1] ^ SET[2];
	unsigned char BCC2 = UA[1] ^ UA[2];

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);


    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	int connected = 0;
	setState current = start;

    int i = 0;
    char message[255];

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);     /* returns after 1 char have been input */
      buf[res]=0;               /* so we can printf... */

	if(connected == 0) 
	switch(current){
		case start: if(buf[0] == F)
						current = flagRCV;
					break;
		case flagRCV:
					if(buf[0] == A)
						current = aRCV;
					else if(buf[0] != F)
						current = start;
				 						break;
		case aRCV:	if(buf[0] == C_SET)
						current = BCC;
					else if(buf[0] != F)
						current = start;
					else 
						current = flagRCV;
					break;
		case cRCV:	if(buf[0]) == BCC1)
						current = BCC;
					else if(buf[0 != F)
						current = start;
					else
						current = flagRCV;
					break;
		case BCC:	if(buf[0] == F)
						current = stop;
					else
						current = start;
					break;
		case stop:	connected = 1;
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

    write(fd, message, strlen(message));

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
