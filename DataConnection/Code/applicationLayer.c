#include "applicationLayer.h"

ApplicationLayer* InitApplication(int port, int status){
  ApplicationLayer *app = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  if (app == NULL)
    return -1;
  app->fileDescriptor = port;
  app->status = status;
  return app;
}

int llopen(ApplicationLayer *app){
  int fd;
  struct termios oldtio,newtio;
  char serialPort[BUF_MAX];

  if((app->status == 0) | (app->status == 1)){
    sprintf(serialPort ,"/dev/ttyS%d", app->fileDescriptor);
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

  return 1;
}

int sendStart(ApplicationLayer* app){
  struct stat fileStat;
  if(stat("../pinguim.gif",&fileStat) < 0)    
      return -1;

  size_t s = fileStat.st_size;
  char* data;
  FILE *file = fopen("../pinguim.gif", "r");
  size_t n = 0;
  int c;

  if (file == NULL) return -1; //could not open file
  fseek(file, 0, SEEK_END);
  long f_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  data = malloc(f_size);

  while ((c = fgetc(file)) != EOF) {
    data[n++] = (char)c;
    printf("%c", c);
  }
  data[n] = '\0';  

  char size[5] = {s/0x100, (s/0x100)/0x100, (s/0x10000)/0x100, s/0x1000000};
  //unsigned char startPackage = {ControlStart, FileSize, 4, size, FILE_NAME};
  //llwrite(startPackage, 7, app);
  return 1;
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