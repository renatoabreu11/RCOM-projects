#include "applicationLayer.h"

int frameCounter = 0;

ApplicationLayer* InitApplication(int port, int status, char * name){
  ApplicationLayer *app = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  if (app == NULL)
    return NULL;
  app->fileDescriptor = port;
  app->status = status;
  app->fileName = name;
  app->nameLength = strlen(app->fileName);
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
  newtio.c_cc[VMIN]     = 100;   /* blocking read until 1 char received */

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

/**
 * If type is equal to 0, sends start, otherwise sends end
 */
int sendStartEnd(ApplicationLayer* app, int type){
  struct stat fileStat;
  if(stat(app->fileName, &fileStat) < 0)
      return -1;

  size_t size = fileStat.st_size;
  int packageLength = app->nameLength + 9;

  char *startPackage = malloc(packageLength);
  if(type == 0)
    startPackage[0] = CONTROL_START;
  else startPackage[1] = CONTROL_START;
  startPackage[1] = FILE_SIZE;
  startPackage[2] = sizeCodified;
  startPackage[3] = size/0x100;
  startPackage[4] = startPackage[3]/0x100;
  startPackage[5] = (size/0x10000)/0x100;
  startPackage[6] = size/0x1000000;
  startPackage[7] = FILE_NAME;
  startPackage[8] = app->nameLength;

  int i = 0;
  for(; i < app->nameLength; i++){
    startPackage[9 + i] = app->fileName[i];
  }

  llwrite(startPackage, packageLength, app);
  free(startPackage);
  return 1;
}

int sendData(ApplicationLayer* app){
  FILE *file = fopen(app->fileName, "r");
  size_t counter = 0;
  int c;

  if (file == NULL) return -1; //could not open file

  char* dataField = malloc(BytesPerPacket);

  while ((c = fgetc(file)) != EOF) {
    dataField[counter++] = (char)c;
    if(counter == BytesPerPacket){
      createDataPackage(dataField, BytesPerPacket, app);
      memset(dataField, 0, BytesPerPacket);
      counter = 0;
      frameCounter++;
    }
  }
  dataField[counter] = '\0';
  createDataPackage(dataField, counter, app);

  free(dataField);
  return 1;
}

int createDataPackage(char * buffer, int length, ApplicationLayer* app){
  char *dataPackage = malloc(length + 4);
  dataPackage[0] = CONTROL_DATA;
  dataPackage[1] = frameCounter;
  dataPackage[2] = L2;
  dataPackage[3] = length;

  int i = 0;
  for(; i < length; i++){
    dataPackage[i+4] = buffer[i];
  }

  llwrite(dataPackage, length + 4, app);
  free(dataPackage);
  return 1;
}

int llwrite(char * buffer, int length, ApplicationLayer* app){
  writeDataFrame(app->fileDescriptor, buffer, length);
  waitForEmissorResponse(app->fileDescriptor, 1);
 	return 1;
}

int llread(char * buffer, ApplicationLayer* app){
  return 1;
}

int llclose(ApplicationLayer* app){
  return 1;
}
