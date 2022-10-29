#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef CHATAPP_CLIENT_H
#define CHATAPP_CLIENT_H

#define PORT 12020

class Client {
private:
    int iSocketFd;
    struct sockaddr_in serv;
    socklen_t iServSize;
public:
    Client();
    int CreateConnection();
    int readUserInput(char** buffer);
    int SendMsg(char* pcBuffer, int iMsgSize);
    int ReceiveACK();
};

#endif //CHATAPP_CLIENT_H
