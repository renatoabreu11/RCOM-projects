#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>

#define SERVICE_READY 		220
#define VALID_USER 			331
#define LOGGED_IN    		230
#define CONNECTION_CLOSED	221
#define PASSIVE				227
#define RETRIEVE			150
#define FILE_RECEIVED		226

struct ftp_data{
	int controlSocketFd;
	int dataSocketFd;
};

int connectSocket(struct ftp_data *ftp, const char *ip, int port);
int ftpConnect(struct ftp_data *ftp, const char* ip, int port);
int ftpLogin(struct ftp_data *ftp, const char *username, const char *password);
int ftpSetPassiveMode(struct ftp_data *ftp);
int ftpDownload(struct ftp_data *ftp, const char *path, const char *filename);
int ftpLogout(struct ftp_data *ftp);
int ftpRead(struct ftp_data *ftp, char *str, size_t size, int expectedCode);
int ftpSendMessage(struct ftp_data * ftp, char *str);