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
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_TRIES 3
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

volatile int STOP=FALSE;
int timer = 1, flag = 1;
typedef enum {start, flagRCV, aRCV, cRCV, BCC, stop}uaState;

void atende()                   {
	printf("alarme # %d\n", timer);
	flag=1;
	timer++;
}

int main(int argc, char** argv)
{
    unsigned char SET[5] = {FLAG, A, C_SET, A^C_SET, FLAG};
	unsigned char UA[5] = {FLAG, A, C_UA, A^C_UA, FLAG};
	(void) signal(SIGALRM, atende);

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
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
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

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

	//Estabilishing connection
	uaState current = start;
	write(fd, SET, 5); //SET packet sent

	int uaReceived = 0;
	int connected = 0;

	int j = 0;			
	for(j; j <= MAX_TRIES; j++){
		if(flag){
  			alarm(3);                 // set 3 seconds alarm
  			flag=0;
		}

		if(timer >= 4){
		
		}else{
			if(connected == 0) 
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
				case aRCV:	if(buf[0] == C_UA)
								current = cRCV;
							else if(buf[0] != FLAG)
								current = start;
							else 
								current = flagRCV;
							break;
				case cRCV:		if(buf[0] == A^C_UA)
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
				case stop:	connected = 1;
							uaReceived = 1;
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
    printf("String received: %s", message);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
