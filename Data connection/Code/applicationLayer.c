#include "applicationLayer.h"

ApplicationLayer* llopen(int port, int status){
  ApplicationLayer * app = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  if (app == NULL)
    return NULL;

  int fd;
  struct termios oldtio,newtio;

  char serialPort[BUF_MAX];

  if((port == 0) | (port == 1)){
    sprintf(serialPort ,"/dev/ttyS%d", port);
  } else{
    printf("Wrong serial port chosen. The port number is zero or one.");
    return NULL;
  }

/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
  fd = open(serialPort, O_RDWR | O_NOCTTY);
  if (fd <0) {
    perror(serialPort); exit(NULL);; 
  }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(NULL);
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
    exit(NULL);
  }

  printf("New termios structure set\n");

  app->fileDescriptor = fd;
  app->status = status;
  return app;
}

int llwrite(char * buffer, int length, ApplicationLayer* app){
  return 1;
}

int llread(char * buffer, ApplicationLayer* app){
  return 1;
}

int llclose(ApplicationLayer* app){
  return 1;
}