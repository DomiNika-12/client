#include <cerrno>
#include "client.h"

Client::Client()
{
    iSocketFd = 0;
    iServSize = sizeof(serv);
    bzero((char *) &serv, iServSize);
}

int Client::readUserInput(char** buffer)
{
    char c = 0;
    int iInitialSize = 5;
    int i = 0;

    *buffer = (char*) malloc(sizeof(char) * 5);
    c = fgetc(stdin);

    while (c != 10)
    {
        if (i > iInitialSize)
        {
            *buffer = (char*) realloc(*buffer, iInitialSize * 2);
            iInitialSize = iInitialSize * 2;
        }
        (*buffer)[i] = c;
        i++;
        c = fgetc(stdin);
        //printf("%c", c);
    }
    if (i > iInitialSize)
    {
        *buffer = (char*) realloc(*buffer, iInitialSize * 2);
        iInitialSize = iInitialSize * 2;
    }
    (*buffer)[i] = '\n';
    i++;
    return i;
}

int Client::CreateConnection()
{
    int iError = 0;

    // Create socket: IPv4 domain, TCP, default protocol
    iSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iSocketFd == -1)
    {
        printf("Error while creating socket (%d) %s \n", errno, strerror(errno));
        iError = iSocketFd;
        return iError;
    }

    int opt = 1;
    setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
//
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    //inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    iError = connect(iSocketFd, (struct sockaddr *) &serv, sizeof(serv));
    if (iError < 0)
    {
        printf("Error while connecting to the socket (%d) %s \n", errno, strerror(errno));
        return iError;
    }
    printf("Client running\n");
    printf("Port:       %d (network byte order)\n", serv.sin_port);
    printf("            %d (hostorder)\n", PORT);
    printf("Domain:     AF_INET\n");
    printf("Protocol:   TCP\n\n");

    return iError;
}



int Client::SendMsg(char* pcBuffer, int iMsgSize, IDHeader header)
{
    int iError = 0;
    int iMsgSizeNBO = 0;
    int iHeaderSize = 0;
    int iHeaderSizeNBO = 0;

    iHeaderSize = sizeof(header);
    // Send self id
    iHeaderSizeNBO = htonl(iMsgSize);
    iError = write(iSocketFd, &iHeaderSizeNBO, 4);
    if (iError < 0)
    {
        printf("Size message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d Request content: %d\n", iError, iHeaderSize);

    iError = write(iSocketFd, &header, sizeof(iMsgSizeNBO));
    if (iError < 0)
    {
        printf("Size message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d Request content: %d\n", iError, iMsgSize);

    // Send message
    iMsgSizeNBO = htonl(iMsgSize);
    iError = write(iSocketFd, &iMsgSizeNBO, sizeof(iMsgSizeNBO));
    if (iError < 0)
    {
        printf("Size message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d Request content: %d\n", iError, iMsgSize);

    iError = write(iSocketFd, pcBuffer, iMsgSize);
    if (iError < 0)
    {
        printf("Content message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d Message content:\n%s\n", iError, pcBuffer);

    return iError;
}

int Client::ReceiveACK()
{
    int iError = 0;
    int iACKSize = 0;
    char* pcACK = nullptr;

    iError = read(iSocketFd, &iACKSize, 4);
    if (iError < 0 || iACKSize == 0)
    {
        printf("ACK message not received");
        return iError;
    }
    printf("Number of bytes: %d Request content: %d\n", iError, ntohl(iACKSize));

    pcACK = (char *) malloc(sizeof(char) * (ntohl(iACKSize) + 1));
    iError = read(iSocketFd, pcACK, iACKSize);
    if (iError < 0)
    {
        printf("ACK message not received");
        return iError;
    }

    printf("Number of bytes: %d Confirmation content:\n%s\n\n", iError, pcACK);
    free(pcACK);
    iACKSize = 0;
    return iError;
}

int Client::ReceiveMsg()
{
    int iError = 0;
    int iMsgSize = 0;
    char* pcMsg = nullptr;

    iError = read(iSocketFd, &iMsgSize, 4);
    if (iError < 0 || iMsgSize == 0)
    {
        printf("ACK message not received");
        return iError;
    }
    printf("Number of bytes: %d Request content: %d\n", iError, ntohl(iMsgSize));

    pcMsg = (char *) malloc(sizeof(char) * (ntohl(iMsgSize) + 1));
    iError = read(iSocketFd, pcMsg, iMsgSize);
    if (iError < 0)
    {
        printf("Message not received");
        return iError;
    }

    printf("Number of bytes: %d Message content:\n%s\n\n", iError, pcMsg);
    free(pcMsg);
    iMsgSize = 0;
    return iError;
}

int Client::ReadMsg(char** ppcMsg, int* piMsgSize)
{
    int iError = 0;
    int iMsgSize = 0;

    iError = read(iSocketFd, piMsgSize, 4);
    if (iError < 0)
    {
        printf("Read failed\n");
    }

    *ppcMsg = (char *) malloc(sizeof(char) * (ntohl(*piMsgSize) + 1));
    iError = read(iSocketFd, *ppcMsg, *piMsgSize);
    if (iError < 0)
    {
        printf("Read failed\n");
    }
}

int Client::SendMsg(char** ppcMsg, int iMsgSize)
{
    int iError = 0;

    iError = send(iSocketFd, &iMsgSize, 4, 0);
    if (iError < 0)
    {
        printf("Size of the message not sent, errno (%d), %s\n", iError, errno, strerror(errno));
        return iError;
    }
    printf("Size of the message sent: %d\n", iMsgSize);

    iError = send(iSocketFd, *ppcMsg, iMsgSize, 0);
    if (iError < 0)
    {
        printf("Message content not sent, errno (%d), %s\n", iError, errno, strerror(errno));
        return iError;
    }
    printf("Message sent: %s\n", *ppcMsg);

    return iError;
}