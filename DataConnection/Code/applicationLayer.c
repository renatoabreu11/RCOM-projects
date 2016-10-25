#include "applicationLayer.h"

ApplicationLayer *app;
int frameCounter = 1;

int InitApplication(int port, int status, char * name, char *baudRate, int packageSize, int retries, int timeout){
  app = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  if (app == NULL)
    return -1;

  initLinkLayer(port, baudRate, BytesPerPacket, retries, timeout);
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
  /*if(endConnection(app->fileDescriptor) == -1){
    llclose(app->fileDescriptor);
    return -1;
  }

  ret = llclose(app->fileDescriptor);
  if(ret == -1){
    return -1;
  }*/
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

  char* dataField = malloc(BytesPerPacket);
  while ((bytesRead = fread(dataField, 1, BytesPerPacket, file)) > 0){
    printf("%s\n", dataField);
    check = sendInformation(dataField, bytesRead);
    if(check != 0){
      return -1;
    }
    frameCounter++;
    memset(dataField, 0, BytesPerPacket);
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
  char *controlPackage = malloc(packageLength);

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

  i = 0;
  for(; i < packageLength; i++){
    printf("%c\n", controlPackage[i]);
  }
  printf("Control package length: %d\n", packageLength);

  if(llwrite(controlPackage, packageLength, app->fileDescriptor) == -1){
    free(controlPackage);
    return -1;
  }
  free(controlPackage);
  return 1;
}

int sendInformation(char * buffer, int length){
  char *dataPackage = malloc(length + 4);
  dataPackage[0] = CONTROL_DATA;
  dataPackage[1] = frameCounter + '0';
  dataPackage[2] = length / 256;
  dataPackage[3] = length % 256;

  int i = 0;
  for(; i < length; i++){
    dataPackage[i+4] = buffer[i];
  }
  if(llwrite(dataPackage, length, app->fileDescriptor) == -1){
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

  llclose(app->fileDescriptor);

  /*FILE *file = fopen(app->fileName, "wb");
  if (file == NULL) {
    printf("Error creating file.\n");
    return 0;
  }

  int bytesRead = 0;
  while (bytesRead < app->fileSize) {
    char* buffer = NULL;
    int length = 0;

    if (receiveInformation(buffer, &length) == -1) {
      printf("Error receiv data packet.\n");
      free(buffer);
      return 0;
    }

    frameCounter++;

    fwrite(buffer, 1, length, file);
    free(buffer);

    bytesRead += length;
  }

  if(receiveControl(CONTROL_END) == -1){
    printf("%s\n", "Error receiving END control packet");
    return -1;
  }
*/
  return 1;
}

int receiveControl(int type){
  char * package = NULL;
  if((package = llread(app->fileDescriptor)) == NULL)
    return -1;

  printf("Tamanho do package = %lu\n", strlen(package));

  /*int j = 0;
  for(; j < strlen(package); j++)
    printf(package[j]);*/

  int index = 0;
  type = package[index];
  if(type == CONTROL_START){
    printf("%s\n", "Control Start received\n");
  }else if(type == CONTROL_END){
    printf("%s\n", "Control end received\n");
  } else return -1;
  index++;

  int nParams = 2; int i = 0;
  char *buffer;
  for(; i < nParams; i++){
    char type = package[index++];
    unsigned char length = package[index++];

    switch(type){
      case FILE_SIZE:
        buffer = (char *)malloc(length);
        memcpy(buffer, &package[index], length);
        sscanf(buffer, "%u", &(app->fileSize));
        break;
      case FILE_NAME:
        buffer = (char *)malloc(length+1);
        memcpy(buffer, &package[index++], length);
        buffer[length] = '\0';
        strcpy(app->fileName, buffer);
        break;
      default:
        return -1;
    }
  }
  return 1;
}

int receiveInformation(char *buffer, int *length){
  char * package = NULL;
  if((package = llread(app->fileDescriptor)) == NULL)
    return -1;

  int C = package[0];
  if(C != CONTROL_DATA){
    printf("%s\n", "wrong C flag ");
    return -1;
  }
  int N = package[1];
  if(N != frameCounter){
    return -1;
  }
  int l2 = package[2];
  int l1 = package[3];

  *length = 256 * l2 + l1;
  memcpy(buffer, &package[4], *length);

  return 1;
}
