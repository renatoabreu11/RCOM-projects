#include "applicationLayer.h"

int llopen(ApplicationLayer *app, int port, int status){
  if (app == NULL)
    return -1;

  int fd;
  struct termios oldtio,newtio;
  char serialPort[BUF_MAX];

  if((port == 0) | (port == 1)){
    sprintf(serialPort ,"/dev/ttyS%d", port);
  } else{
    printf("Wrong serial port chosen. The port number is zero or one.");
    return -1;
  }

/*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
  fd = open(serialPort, O_RDWR | O_NOCTTY);
  if (fd <0) {
    perror(serialPort);
    return -1;
  }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
     return -1;
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
     return -1;
  }

  printf("New termios structure set\n");

  app->fileDescriptor = fd;
  app->status = status;

  //TRANSMITTER
  if(app->status == 0)
    connectTransmitter(app->fileDescriptor);
  else
    connectReceiver(app->fileDescriptor);

  return 1;
}

int llwrite(char * buffer, int length, ApplicationLayer* app){

    /*if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);*/
 	return 1;
}

int llread(char * buffer, ApplicationLayer* app){
  return 1;
}

int llclose(ApplicationLayer* app){
  return 1;
}

char readFile(char* in_filepath){
	/*FILE* in;
	unsigned char* buf;
	long size;

	in = fopen(in_filepath, "rb");
	fseek(ptr,0,SEEK_END);
	size = ftell(in);
	buf = (char*)malloc(size+1);
	fseek(in,0,SEEK_SET);
	fread(buf,1,size,in);
	fclose(in);
	buf[size] = NULL;
	return buf;*/
  return 'a';
}

void writeFile(char* out_filepath, char* buf){
	/*FILE* out = fopen(out_filepath,"wb");

	for(int i = 0; buf[i] != NULL; i++){
		fputc(buf[i],out);
	}

	fclose(out);*/
}
