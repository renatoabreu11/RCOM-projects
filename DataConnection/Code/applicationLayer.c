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
  app->link = (LinkLayer*) malloc(sizeof(LinkLayer));
  app->link = InitLink();

  return app;
}

int startConnection(ApplicationLayer *app){
  llopen(app);

  if(app->status == 0){
    connectTransmitter(app->fileDescriptor, app->link);
    //sendData(app);
    disconnectTransmitter(app->fileDescriptor, app->link);
  }else{
    connectReceiver(app->fileDescriptor);
    //getData(app);
    disconnectReceiver(app->fileDescriptor, app->link);
  }
  return 1;
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

  app->fileDescriptor = fd;
  printf("Serial port descriptor: %d\n", app->fileDescriptor);

  return 1;
}

/**
 * If type is equal to 0, sends start, otherwise sends end
 */
char* createStartEnd(ApplicationLayer* app, int type){
  struct stat fileStat;
  if(stat(app->fileName, &fileStat) < 0)
      return NULL;

  size_t size = fileStat.st_size;
  int packageLength = app->nameLength + 9;

  char *startPackage = malloc(packageLength);
  if(type == 0)
    startPackage[0] = CONTROL_START;
  else startPackage[0] = CONTROL_END;
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
  return startPackage;
}

char* createDataPackage(char * buffer, int length, ApplicationLayer* app){
  char *dataPackage = malloc(length + 4);
  dataPackage[0] = CONTROL_DATA;
  dataPackage[1] = frameCounter;
  dataPackage[2] = L2;
  dataPackage[3] = length;

  int i = 0;
  for(; i < length; i++){
    dataPackage[i+4] = buffer[i];
  }

  return dataPackage;
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
      char * startPackage = createStartEnd(app, 0);
      char * dataPackage = createDataPackage(dataField, BytesPerPacket, app);
      char * endPackage = createStartEnd(app, 1);
      concatPackages(startPackage, dataPackage, endPackage, app);
      memset(dataField, 0, BytesPerPacket);
      counter = 0;
      frameCounter++;
    }
  }
  dataField[counter] = '\0';
  char * startPackage = createStartEnd(app, 0);
  char * dataPackage = createDataPackage(dataField, BytesPerPacket, app);
  char * endPackage = createStartEnd(app, 1);
  concatPackages(startPackage, dataPackage, endPackage, app);

  free(dataField);
  return 1;
}

int getData(ApplicationLayer *app) {
  char *buffer = malloc(BytesPerPacket);
  int timesRead = 0;
  FILE *file = fopen("../pinguinCopied.gif", "w");

  do {
    realloc(buffer, BytesPerPacket + timesRead * BytesPerPacket);
    timesRead++;
  } while(llread(buffer, app) > 0);

  //In order to keep the size as best as possible
  timesRead--;
  realloc(buffer, BytesPerPacket + timesRead * BytesPerPacket);
  int bufferLength = BytesPerPacket + timesRead * BytesPerPacket;

  int i = 0;
  for(; i < bufferLength; i++)
    fprintf(file, "%c", buffer[i]);

  free(buffer);
  fclose(file);
}

void concatPackages(char *startPackage, char* dataPackage, char*endPackage, ApplicationLayer* app){
  int newSize = strlen(startPackage)  + strlen(dataPackage) +  strlen(endPackage) +1;

   // Allocate new buffer
   char * newBuffer = (char *)malloc(newSize);

   // do the copy and concat
   strcpy(newBuffer,startPackage);
   strncat(newBuffer,dataPackage, strlen(dataPackage)); // or strncat
   strncat(newBuffer,endPackage, strlen(endPackage));

   // release old buffer
   free(startPackage);
   free(dataPackage);
   free(endPackage);

   llwrite(newBuffer, strlen(newBuffer), app);
}

int llwrite(char * buffer, int length, ApplicationLayer* app){
  writeDataFrame(app->fileDescriptor, buffer, length);
  waitForResponse(app->fileDescriptor, 1, app->link);
 	return 1;
}

int llread(char * buffer, ApplicationLayer* app){
  //No buffer fica a imagem

  //TODO: llread needs to return number of character read
  return readDataFrame(app->fileDescriptor, buffer);
}

int llclose(ApplicationLayer* app){
  /*if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }*/
  return 1;
}
