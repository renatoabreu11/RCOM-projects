#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX

#include "applicationLayer.h"

int baudRate = 38400;
int packageLength = 1024;
int retries = 3;
int timeOut = 3;

void clear_stream(FILE *in){
	int ch;

	clearerr(in);

	do
	ch = getc(in);
	while (ch != '\n' && ch != EOF);

	clearerr(in);
}

int chooseParameter(char *type){
	char parameter[30];
  	sprintf(parameter,"\nEnter %s: ", type);
	int value;

    printf("%s", parameter);
    fflush(stdout);

    while (scanf("%d", &value) != 1) {
        clear_stream(stdin);
        printf("Invalid integer. Please try again: ");
        fflush(stdout);
    }
	return value;
}

void checkBaudRateValue(){
	if(baudRate != 4800 || baudRate != 9600 || baudRate != 19200 || baudRate != 38400 || baudRate != 57600){
		printf("%s\n", "Invalid baud rate value. B38400 defined as default");
		baudRate = 38400;
	}
}

void showMenu(int port, int status){
	char option;

	printf("\n1 - Start Connection\n");
	printf("2 - Baud rate\n");
	printf("3 - Data package length\n");
	printf("4 - Retries\n");
	printf("5 - Time out\n");
	printf("6 - Quit\n");
	printf("\nChoose your option: ");

  	do{
  		scanf("%c",&option);

 		switch(option){
  		case'1':
  			printf("\n\n");
  			char baud[10];
  			sprintf(baud,"B%d", baudRate);
  			InitApplication(port, status, "../hello.txt", baud, packageLength, retries, timeOut);
  			break;
  		case'2':
  			baudRate = chooseParameter("Baud rate");
  			checkBaudRateValue();
  			clear_stream(stdin);
  			showMenu(port, status);
  			return;
  		case'3':
  			packageLength = chooseParameter("Data package Length");
  			clear_stream(stdin);
  			showMenu(port, status);
  			return;
  		case'4':
  			retries = chooseParameter("number of retries");
  			clear_stream(stdin);
  			showMenu(port, status);
  			return;
  		case'5':
  			timeOut = chooseParameter("Time out interval");
  			clear_stream(stdin);
  			showMenu(port, status);
  			return;
  		case'6':
  			exit(0);
  		default:
     	 	printf("invalid input, please type again:");
  		}
  }while(option<'1' ||option>'6');
}

int main(int argc, char **argv) {
	if ( (argc < 3) ||
		((strcmp("0", argv[1])!=0) &&
	(strcmp("1", argv[1])!=0) ) ||
	((strcmp("0", argv[2])!=0) &&
	(strcmp("1", argv[2])!=0) )) {
		printf("Usage:\t./main Port Status\n\tex: ./main 0 1\n");
		exit(1);
	}

	char *p, *c;
	int port, status;

	errno = 0;
	long conv = strtol(argv[1], &p, 10);

	if (errno != 0 || *p != '\0' || conv > INT_MAX) {
		printf("Invalid port value. It must be a valid int.\n");
		return -1;
	} else {
		port = conv;
	}

	errno = 0;
	conv = strtol(argv[2], &c, 10);
	if (errno != 0 || *c != '\0' || conv > INT_MAX) {
		printf("Invalid status value. It must be a valid int.\n");
		return -1;
	} else {
		status = conv;
	}

	showMenu(port, status);
	return 1;
}
