#include "ftp.h"

int connectSocket(struct ftp_data *ftp, const char *ip, int port){
	struct	sockaddr_in server_addr;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((ftp->controlSocketFd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        return -1;
    }

	/*connect to the server*/
    if(connect(ftp->controlSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		return -1;
	}

    return 1;
}

/*
 * Connect socket (control socket) to ftp server 
 * Receive a reply from the ftp server (code : 220).
 */
int ftpConnect(struct ftp_data *ftp, const char *ip, int port){	
	if(connectSocket(ftp, ip, port) == -1)
		return -1;

	char str[1024];
	if(ftpRead(ftp, str, sizeof(str)) == -1) {
		printf("Error: couldn't read the server's response!");
		return -1;
	}

	char code[4];
	memcpy(code, str, 3);
	code[3] = '\0';

	if(strcmp(code, "220") != 0) {
		printf("Error: wrong code received!");
		return -1;
	}

	return 1;
}

/*
 * Send login to the ftp server using the command USER and wait for confirmation (331)
 * Send password using the command PASS and wait for confirmation (230). 
 */
int ftpLogin(struct ftp_data *ftp, const char *username, const char *password){
	char message[1024];
	char response[1024];
	char code[4];

	/********** Sends USER **********/

	//'\r' is to simulate 'ENTER'
	sprintf(message, "USER %s\r\n", username);

	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, strlen(response)) == -1) {
		printf("Error: couldn't read the server's response!");
		return -1;
	}

	memcpy(code, response, 3);
	code[3] = '\0';

	if(strcmp(code, "331") != 0) {
		printf("Error: wrong code received!");
		return -1;
	}

	/********** Sends PASS **********/
	
	memset(response, 0, strlen(response));
	memset(code, 0, strlen(code));
	sprintf(message, "pass %s\r\n", password);

	if(ftpSendMessage(ftp, message) == -1)
		return -1;
	if(ftpRead(ftp, response, strlen(response)) == -1) {
		printf("Error: couldn't read the server's response!");
		return -1;	
	}

	memcpy(code, response, 3);
	code[3] = '\0';
	if(strcmp(code, "230") != 0) {
		printf("Error: wrong code received!");
		return -1;
	}

	return 1;
}

/* 
 * Send command PASV, and waits for a reply that gives an IP address and a port
 * Parse this message,
 * Connect a second socket (a data socket) with the given configuration.
 */
int ftpSetPassiveMode(struct ftp_data *ftpData){
	return 1;
}

/*
 * Send the RETR command with the file path
 */
int ftpDownload(struct ftp_data *ftpData, const char *path){
	return 1;
}

/*
 * Send the QUIT command and wait for reply (221)
 */ 
int ftpLogout(struct ftp_data *ftpData){
	char message[1024];
	char response[1024];
	char code[4];

	sprintf(message, "QUIT\r\n");
	if(ftpSendMessage(ftpData, message) == -1)
		return -1;
	if(ftpRead(ftpData, response, sizeof(response))) {
		printf("Error: couldn't read the server's response!");
		return -1;
	}

	memcpy(code, response, 3);
	code[3] = '\0';

	if(strcmp(code, "221") != 0) {
		printf("Error: wrong code received!");
		return -1;
	}

	return 1;
}

int ftpRead(struct ftp_data *ftpData, char *str, size_t size) {
	FILE *file = fdopen(ftpData->controlSocketFd, "r");

	if(fgets(str, size, file) == NULL)
		return -1;

	return 1;
}

int ftpSendMessage(struct ftp_data * ftpData, char *str){
	int strLength = strlen(str);
	
	if(write(ftpData->controlSocketFd, str, strLength) != strLength){
		printf("%s\n", "Error writing to ftp server");
		return -1;
	}

	return 1;
}