//
// Created by Dominika Bobik on 10/29/22.
//

#include "client.h"

int main(int argc, char *argv[]) {

    int iError = 0;
    char* pcMsg = nullptr;
    int iMsgSize = 0;
    auto* client = new Client();

    iError = client->CreateConnection();
    if (iError != 0)
    {
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter your message:\n");
        client->readUserInput(&pcMsg);
        iMsgSize = strlen(pcMsg);
        iError = client->SendMsg(pcMsg, iMsgSize);
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }

        iError = client->ReceiveACK();
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}