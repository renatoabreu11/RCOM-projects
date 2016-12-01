#include "ftp.h"

int connectSocket(struct ftp_data *ftp, const char *ip, int port){
	int sockfd;
	struct	sockaddr_in server_addr;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        return -1;
    }

	/*connect to the server*/
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		return -1;
	}

    return sockfd;
}

/*
 * Connect socket (control socket) to ftp server 
 * Receive a reply from the ftp server (code : 220).
 */
int ftpConnect(struct ftp_data *ftp, const char *ip, int port){	
	if((ftp->controlSocketFd = connectSocket(ftp, ip, port)) == -1)
		return -1;

	char response[1024];
	if(ftpRead(ftp, response, sizeof(response), SERVICE_READY) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);

	return 1;
}

/*
 * Send login to the ftp server using the command USER and wait for confirmation (331)
 * Send password using the command PASS and wait for confirmation (230). 
 */
int ftpLogin(struct ftp_data *ftp, const char *username, const char *password){
	char message[1024];
	char response[1024];

	/********** Sends USER **********/

	//'\r' is to simulate 'ENTER'
	sprintf(message, "USER %s\r\n", username);

	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, strlen(response), VALID_USER) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);

	/********** Sends PASS **********/
	
	memset(response, 0, sizeof(response));
	sprintf(message, "PASS %s\r\n", password);
	printf("%s", message);

	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, sizeof(response), LOGGED_IN) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);

	return 1;
}

/* 
 * Send command PASV, and waits for a reply that gives an IP address and a port
 * Parse this message,
 * Connect a second socket (a data socket) with the given configuration.
 */
int ftpSetPassiveMode(struct ftp_data *ftp){
	char ip[1024];
	int port;

	char message[1024] = "PASV\r\n";
	if(ftpSendMessage(ftp, message) == -1)
		return -1;

	memset(message, 0, sizeof(message));
	if(ftpRead(ftp, message, sizeof(message), PASSIVE) == -1) {
		printf("Error: %s\n", message);
		return -1;	
	}

	printf("Passive message = %s", message);

	//Parses the message received
	int ipPart1, ipPart2, ipPart3, ipPart4, portPart1, portPart2;
	sscanf(message, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1, &ipPart2, &ipPart3, &ipPart4, &portPart1, &portPart2);

	//Builds IP
	sprintf(ip, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);
	//Builds Port
	port = portPart1 * 256 + portPart2;

	printf("IP = %s, port = %d\n", ip, port);

	if((ftp->dataSocketFd = connectSocket(ftp, ip, port)) == -1) {
		printf("Error: couldn't connect to socket on PASV\n");
		return -1;
	}

	return 1;
}

/*
 * Send the RETR command with the file path
 */
int ftpDownload(struct ftp_data *ftp, const char *path, const char *filename){
	char message[1024];
	char response[1024];

	sprintf(message, "RETR %s\r\n", path);

	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, sizeof(response), RETRIEVE) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);

	FILE *file = fopen(filename, "wb");

	int bytesRead;
	memset(message, 0, sizeof(message));
	while((bytesRead = read(ftp->dataSocketFd, message, sizeof message)) > 0){
		fwrite(message, 1, bytesRead, file);
	}
	//up201403377@fe.up.pt

	fclose(file);

	memset(response, 0, sizeof(response));
	if(ftpRead(ftp, response, sizeof(response), FILE_RECEIVED) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);
	return 1;
}

/*
 * Send the QUIT command and wait for reply (221)
 */ 
int ftpLogout(struct ftp_data *ftp){
	char message[1024];
	char response[1024];

	sprintf(message, "QUIT\r\n");
	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, sizeof(response), CONNECTION_CLOSED) == -1) {
		printf("Error: %s!", response);
		return -1;
	}
	printf("%s", response);

	return 1;
}

int ftpRead(struct ftp_data *ftp, char *str, size_t size, int expectedCode) {
	FILE *file = fdopen(ftp->controlSocketFd, "r");
	char code[4];
	int codeInt;
	do{
		memset(code, 0, sizeof code);
		memset(str, 0, size);
		str = fgets(str, size, file);
		memcpy(code, str, 3);
		code[3] = '\0';
		codeInt = atoi(code);
	}while(codeInt < 100 || codeInt >= 600);

	if(expectedCode != 0 && codeInt != expectedCode){
		return -1;
	}

	return 1;
}

int ftpSendMessage(struct ftp_data * ftp, char *str){
	int strLength = strlen(str);
	
	if(write(ftp->controlSocketFd, str, strLength) != strLength){
		printf("%s\n", "Error writing to ftp server");
		return -1;
	}

	return 1;
}