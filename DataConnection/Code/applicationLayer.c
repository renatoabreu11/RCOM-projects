#include "applicationLayer.h"

ApplicationLayer *app;
int frameCounter = 1;

int InitApplication(int port, int status, char * name, int baudRate, int packageSize, int retries, int timeout){
  app = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  if (app == NULL)
  return -1;

  initLinkLayer(port, baudRate, packageSize, retries, timeout);
  int ret = llopen(status, port);
  if(ret == -1){
    return -1;
  } else{
    app->fileDescriptor = ret;
  }
  app->status = status;

  if(app->status == 0){
    app->fileName = name;
    app->nameLength = strlen(app->fileName);

    struct stat fileStat;
    if (stat(app->fileName, &fileStat) == 0)
    app->fileSize = fileStat.st_size;
    else{
      printf("%s\n", "Error getting file size");
      return -1;
    }
  }

  if(estabilishConnection(app->fileDescriptor) == -1){
    llclose(app->fileDescriptor);
    return -1;
  }
  if(startConnection() == -1){
    llclose(app->fileDescriptor);
    return -1;
  }
  if(endConnection(app->fileDescriptor) == -1){
    llclose(app->fileDescriptor);
    return -1;
  }

  ret = llclose(app->fileDescriptor);
  if(ret == -1){
    return -1;
  }
  return 1;
}

int startConnection(){
  if(app->status == TRANSMITTER){
    if(sendData() == -1){
      printf("%s\n", "Error sending file data");
      return -1;
    }
  }else if(app->status == RECEIVER){
    if(receiveData() == -1){
      printf("%s\n", "Error receiving file data");
      return -1;
    }
  }
  return 1;
}

int sendData(){
  FILE *file = fopen(app->fileName, "rb");
  int bytesRead = 0;

  if (file == NULL) return -1;

  int check = sendControl(CONTROL_START);
  if(check == -1){
    printf("%s\n", "Error sending START packet");
    llclose(app->fileDescriptor);
    return -1;
  }


  unsigned char* dataField = malloc(DataLength);
  while ((bytesRead = fread(dataField, 1, DataLength, file)) > 0){
    check = sendInformation(dataField, bytesRead);
    if(check == -1){
      return -1;
    }
    frameCounter++;
    memset(dataField, 0, DataLength);
  }
  fclose(file);
  free(dataField);

  check = sendControl(CONTROL_END);
  if(check == -1){
    printf("%s\n", "Error sending END packet");
    llclose(app->fileDescriptor);
    return -1;
  }
  return 1;
}

int sendControl(int type){
  char fileSizeStr[20];
  sprintf(fileSizeStr,"%u",app->fileSize);

  int packageLength = app->nameLength + strlen(fileSizeStr) + 5;
  unsigned char *controlPackage = malloc(packageLength);

  int index = 0;
  controlPackage[index] = type;
  index++;
  controlPackage[index] = FILE_SIZE;
  index++;
  controlPackage[index] = strlen(fileSizeStr) + '0';

  index++;
  int i = 0;
  for(; i < strlen(fileSizeStr); i++){
    controlPackage[index] = fileSizeStr[i];
    index++;
  }

  controlPackage[index] = FILE_NAME;
  index++;
  controlPackage[index] = app->nameLength + '0';
  index++;

  i = 0;
  for(; i < app->nameLength; i++){
    controlPackage[index] = app->fileName[i];
    index++;
  }

  if(llwrite(controlPackage, packageLength, app->fileDescriptor) == -1){
    free(controlPackage);
    return -1;
  }
  free(controlPackage);
  return 1;
}

int sendInformation(unsigned char * buffer, int length){
  int dataPacketLength = length + DataHeaders;
  unsigned char *dataPackage = malloc(dataPacketLength);
  dataPackage[0] = CONTROL_DATA;
  dataPackage[1] = frameCounter ;
  dataPackage[2] = length / 256 ;
  dataPackage[3] = length % 256;

  int i = 0;
  for(; i < length; i++){
    dataPackage[i+DataHeaders] = buffer[i];
  }

  if(llwrite(dataPackage, dataPacketLength, app->fileDescriptor) == -1){
    free(dataPackage);
    return -1;
  }
  free(dataPackage);
  return 1;
}

int receiveData(){
  if(receiveControl(CONTROL_START) == -1){
    printf("%s\n", "Error receiving control package");
    return -1;
  }

  FILE *file = fopen("pinguim.gif", "wb");
  //FILE *file = fopen(app->fileName, "wb");
  if (file == NULL) {
    printf("Error creating file.\n");
    return 0;
  }

  unsigned char* buffer = malloc(DataLength);
  int bytesRead = 0;

  while (bytesRead < app->fileSize) {
    int length = 0;

    if (receiveInformation(buffer, &length) == -1) {
      printf("Error receiv data packet.\n");
      free(buffer);
      return 0;
    }

    frameCounter++;

    fwrite(buffer, 1, length, file);

    memset(buffer, 0, DataLength);

    bytesRead += length;
    printf("Bytes read: %d\n", bytesRead);
  }

  if(receiveControl(CONTROL_END) == -1){
    printf("%s\n", "Error receiving END control packet");
    return -1;
  }

  fclose(file);

  return 1;
}

int receiveControl(int control){
  unsigned char *package = malloc(DataLength);
  int packageSize;
  if((packageSize = llread(app->fileDescriptor, package)) == -1)
    return -1;
  int j = 0;
  printf("Package size %d\n", packageSize);
  for(; j < packageSize; j++){
    printf("%c\n", package[j]);
  }

  int index = 0;
  control = package[index++];
  if(control == CONTROL_START){
    printf("%s\n", "Control Start received");
  }else if(control == CONTROL_END){
    printf("%s\n", "Control end received");
  } else return -1;

  int nParams = 2; int i = 0;
  int length;

  for(; i < nParams; i++){
    int type = package[index++];
    length = (char)package[index++] - '0';
    switch(type) {
      case FILE_SIZE:{
       char *fileLength = (char *)malloc(length);
       memcpy(fileLength, &package[index], length);
       sscanf(fileLength, "%u", &(app->fileSize));
       free(fileLength);
       index += length;
       break;
     }
     case FILE_NAME:{
       app->fileName = (char *)malloc(length+1);
       memcpy(app->fileName, &package[index], length);
       app->fileName[length] = '\0';
       break;
     }
     default: break;
   }
 }
  return 1;
}

int receiveInformation(unsigned char *buffer, int *length){
  unsigned char *package = malloc(DataLength);
  int packageSize;
  if((packageSize = llread(app->fileDescriptor, package)) == -1)
    return -1;

  printf("N = %c\n", package[0]);

  int C = package[0];
  if(C != CONTROL_DATA){
    printf("%s\n", "wrong C flag ");
    return -1;
  }

  int N = package[1];
  if(N != frameCounter){
    printf("%s\n", "neh");
    return -1;
  }

  int l2 = package[2];
  int l1 = package[3];

  printf("l2 = %d\n", l2);
  printf("l1 = %d\n", l1);

  *length = 256 * l2 + l1;
  printf("Length = %d\n\n\n", *length);
  memcpy(buffer, &package[4], *length);

  printf("Tamanho do package = %d\n", packageSize);

  free(package);

  /*int i;
  for(i = 0; i < length; i++)
  printf("Byte lido da imagem = %c\n", buffer[i]);*/

  return 1;
}
