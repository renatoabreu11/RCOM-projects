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
    check = sendInformation(dataField, bytesRead);
    if(check == -1){
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
  dataPackage[2] = length / 256 + '0';
  dataPackage[3] = length % 256 + '0';

  int i = 0;
  for(; i < length; i++){
    dataPackage[i+4] = buffer[i];
  }
  i = 0;
  for(; i < length + 4; i++){
    printf("%c\n", dataPackage[i]);
  }
  if(llwrite(dataPackage, length + 4, app->fileDescriptor) == -1){
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

  //CHANGE THIS HARD CODED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  FILE *file = fopen("hello.txt", "wb");
  if (file == NULL) {
    printf("Error creating file.\n");
    return 0;
  }

  char* buffer = malloc(1000);
  int bytesRead = 0;

  while (bytesRead < 58) {
    //Change this HARD CODED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int length = 0;

    printf("Starting to receive information\n");

    if (receiveInformation(buffer, &length) == -1) {
      printf("Error receiv data packet.\n");
      free(buffer);
      return 0;
    }

    printf("Received 5 bytes with sucess!\n");

    frameCounter++;

    fwrite(buffer, 1, length, file);

    printf("Starting freeing buffer!\n");
    //free(buffer);
    memset(buffer, 0, 1000);
    printf("Freed buffer!\n");

    bytesRead += length;
  }

  if(receiveControl(CONTROL_END) == -1){
    printf("%s\n", "Error receiving END control packet");
    return -1;
  }

  fclose(file);

  return 1;
}

int receiveControl(int control){
  char *package = NULL;
  if((package = llread(app->fileDescriptor)) == NULL)
  return -1;

  printf("Package recebido\n");
  int j;
  for(j = 0; j < strlen(package); j++)
  printf("%c\n", package[j]);

  int index = 0;
  control = package[index++];
  if(control == CONTROL_START){
    printf("%s\n", "Control Start received\n");
  }else if(control == CONTROL_END){
    printf("%s\n", "Control end received\n");
  } else return -1;

  int nParams = 2; int i = 0;
  char *buffer;
  for(; i < nParams; i++){
    unsigned int type = package[index++];
    printf("Type = %u\n", type);
    int length = package[index++] - '0';
    printf("Length = %d\n", length );

    switch(type) {
      case FILE_SIZE:
      printf("FODASSE 2 2 2 2 2 \n");
      buffer = (char *)malloc(length);
      printf("Before memcpy\n");
      memcpy(buffer, &package[index], length);
      printf("Em memcpy\n");
      //sscanf(buffer, "%u", &(app->fileSize));
      printf("Em sscanf\n");
      printf("%u\n", app->fileSize );
      break;
      case FILE_NAME:
      printf("FODASSE\n");
      buffer = (char *)malloc(length+1);
      memcpy(buffer, &package[index++], length);
      buffer[length] = '\0';
      strcpy(app->fileName, buffer);
      printf("%s\n", app->fileName);
      break;
      default: break;
    }
  }

  return 1;
}

int receiveInformation(char *buffer, int *length){
  char * package = NULL;

  if((package = llread(app->fileDescriptor)) == NULL)
    return -1;

  printf("N = %c\n", package[0]);

  int C = package[0];
  if(C != CONTROL_DATA){
    printf("%s\n", "wrong C flag ");
    return -1;
  }

  int N = package[1] - '0';
  if(N != frameCounter){
    return -1;
  }

  int l2 = package[2] - '0';
  int l1 = package[3] - '0';

  printf("l2 = %d\n", l2);
  printf("l1 = %d\n", l1);

  *length = 256 * l2 + l1;
  printf("Length = %d\n\n\n", *length);
  memcpy(buffer, &package[4], *length);

  printf("Tamanho do package = %lu\n", strlen(package));

  free(package);

  /*int i;
  for(i = 0; i < length; i++)
  printf("Byte lido da imagem = %c\n", buffer[i]);*/

  return 1;
}
