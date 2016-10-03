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

int main(int argc, char** argv)
{
    
    unsigned char SET[5];
	SET[0] = FLAG;
	SET[1] = A;
	SET[2] = C_SET;
	SET[3] = A^C_SET;
	SET[4] = FLAG;
	unsigned char UA[5];
	UA[0] = FLAG;
	UA[1] = A;
	UA[2] = C_UA;
	UA[3] = A^C_UA;
	UA[4] = FLAG;
    
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

    newtio.c_cc[VTIME]    = 30;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

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

    for(i=0;i<MAX_TRIES&&strcmp(buf,UA)!=0;i++){
	printf("Before write\n"); //DBG
	res=write(fd,SET,5);
	printf("After write\n"); //DBG
	res=read(fd,buf,5);
	buf[res]=0;
	printf("After read\n"); //DBG
	}

	if(i>=3) {
		printf("Exceeded number of connection attempts\n");
		return 1;
	}
    
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
