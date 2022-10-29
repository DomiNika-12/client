#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cerrno>

// Chat CLI app
// Client

int iSocketFd;
struct sockaddr_in serv;
socklen_t iServSize;

int readUserInput (char** buffer) {
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

int InitializeParameters()
{
    int iError = 0;

    iSocketFd = 0;
    iServSize = sizeof(serv);
    bzero((char *) &serv, iServSize);

    return iError;
}

int CreateConnection()
{
    int iError = 0;
    int iPort = 0;

    // Create socket: IPv4 domain, UDP, default protocol
    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd == -1)
    {
        printf("Error while creating socket\n");
        iError = iSocketFd;
        return iError;
    }

    iPort = 12020;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(iPort);

    printf("Client running\n");
    printf("Port:       %d (network byte order)\n", serv.sin_port);
    printf("            %d (hostorder)\n", iPort);
    printf("Domain:     AF_INET\n");
    printf("Protocol:   UDP\n\n");
    return iError;
}

int SendMsg(char* pcBuffer, int iMsgSize)
{
    int iError = 0;
    int iMsgSizeNBO = 0;

    iMsgSizeNBO = htonl(iMsgSize);
    iError = sendto(iSocketFd, &iMsgSizeNBO, sizeof(iMsgSizeNBO), 0, (struct sockaddr *) &serv, iServSize);
    if (iError < 0)
    {
        printf("Size message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d\n Request content:\n%d\n", iError, iMsgSize);

    iError = sendto(iSocketFd, pcBuffer, iMsgSize, 0, (struct sockaddr *) &serv, iServSize);
    if (iError < 0)
    {
        printf("Content message not sent, error (%d)\n", iError);
        return iError;
    }
    printf("Number of bytes: %d\n Message content:\n%s\n", iError, pcBuffer);

    return iError;
}

int ReceiveACK()
{
    int iError = 0;
    int iACKSize = 0;
    char* pcACK = nullptr;

    iError = recvfrom(iSocketFd, &iACKSize, 4, 0, (struct sockaddr *) &serv, &iServSize);
    if (iError < 0 || htonl(iACKSize) == 0)
    {
        printf("ACK message not received");
        return iError;
    }
    printf("Number of bytes: %d\n Request content:\n%d\n", iError, htonl(iACKSize));

    pcACK = (char*)(malloc(sizeof(char) * (htonl(iACKSize) + 1)));
    iError = recvfrom(iSocketFd, (char *) pcACK, iACKSize, 0, (struct sockaddr *) &serv, &iServSize);
    if (iError < 0)
    {
        printf("ACK message not received");
        return iError;
    }

    printf("Number of bytes: %d\n Confirmation content:\n%s\n\n", iError, pcACK);
    free(pcACK);
    return iError;
}

int main(int argc, char *argv[]) {

    int iError = 0;
    char* pcMsg = nullptr;
    int iMsgSize = 0;

    iError = InitializeParameters();
    if (iError != 0)
    {
        exit(EXIT_FAILURE);
    }

    iError = CreateConnection();
    if (iError != 0)
    {
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter your message:\n");
        readUserInput(&pcMsg);
        iMsgSize = strlen(pcMsg);
        iError = SendMsg(pcMsg, iMsgSize);
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }

        iError = ReceiveACK();
        if (iError < 0)
        {
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}