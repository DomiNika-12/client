//
// Created by Dominika Bobik on 10/29/22.
//

#include <thread>
#include "client.h"

Client* client;

void ReadThread()
{
    int iError = 0;
    char* pcMsg = nullptr;
    int iMsgSize = 0;

    while (1) {
        iError = client->ReadMsg(&pcMsg, &iMsgSize);
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }
        printf("----: %s \n", pcMsg);
    }
}

void SendThread()
{
    int iError = 0;
    char* pcMsg = nullptr;
    int iMsgSize = 0;

    while (1) {
        fflush(stdin);
        client->readUserInput(&pcMsg);
        iMsgSize = strlen(pcMsg);

        iError = client->SendMsg(&pcMsg, iMsgSize);
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {

    int iError = 0;
    char* pcMsg = nullptr;
    int iMsgSize = 0;
    client = new Client();

    iError = client->CreateConnection();
    if (iError != 0)
    {
        exit(EXIT_FAILURE);
    }

    // Construct ID header from user input
    struct IDHeader header;
    printf("Enter your ID (1 char):\n");
    header.selfID = getc(stdin);
    getc(stdin);
    printf("Enter ID of the client you'd like to talk to (1 char):\n");
    header.clientID = getc(stdin);
    getc(stdin);

    // Send header
    iError = send(client->iSocketFd, &header, sizeof(header), 0);
    if (iError < 0)
    {
        printf("Failed to send a header, errno (%d)\n", errno);
        exit(EXIT_FAILURE);
    }
    // Start read and send threads
    std::thread tRead(&ReadThread);
    std::thread tSend(&SendThread);
    tRead.join();
    tSend.join();

    return 0;
}