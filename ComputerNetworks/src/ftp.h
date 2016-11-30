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

struct ftp_data{
	int controlSocketFd;
	int dataSocketFd;
};

int connectSocket(struct ftp_data *ftp, const char *ip, int port);
int ftpConnect(struct ftp_data *ftp, const char* ip, int port);
int ftpLogin(struct ftp_data *ftp, const char *username, const char *password);
int ftpSetPassiveMode(struct ftp_data *ftpData);
int ftpDownload(struct ftp_data *ftpData, const char *path);
int ftpLogout(struct ftp_data *ftpData);
int ftpSendMessage(struct ftp_data * ftpData, char *str, int bytes);