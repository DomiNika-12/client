#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Client
// Communication with sockets through UDP protocol

int main(int argc, char* argv[]) 
{
    if (argc != 2) 
	{ 
		printf("Wrong number of arguments provided\n"); 
		exit(EXIT_FAILURE); 
	}

	struct hostent* host = nullptr;
	struct sockaddr_in serv = {};
	bzero((char *) &serv, sizeof(serv));

	// Create socket: IPv4 domain, UDP, default protocol
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1)
	{
		printf("Error while creating socket\n");
		exit(EXIT_FAILURE);
	}

	if ((host = ::gethostbyname("localhost")) == nullptr)
	{
		printf("Invalid hostname: localhost\n");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	u_short port = 12020; // Port in nbo

	// Server info
	serv.sin_family = AF_INET;
	bcopy(host->h_addr_list[0], (char *)&serv.sin_addr, host->h_length);
	serv.sin_port = port;

	// Send request to the server
	char* symbolic_hostname = static_cast<char *>(malloc(sizeof(char) * (strlen(host->h_name) + 1)));
	strcpy(symbolic_hostname, 	host->h_name);
	int size = (int)strlen(symbolic_hostname);
	printf("Sending hostname <%s>\n", symbolic_hostname);
	fd_set mask;
	struct timeval timeout;
	FD_ZERO(&mask);
	FD_SET(socket_fd, &mask);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	sendto(socket_fd, symbolic_hostname, size, 0, (struct sockaddr *)&serv, sizeof(serv));
	int retval = select(socket_fd + 1, &mask, NULL, NULL, &timeout);
	if (retval == 0)
	{
		FD_ZERO(&mask);
		FD_SET(socket_fd, &mask);
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		sendto(socket_fd, symbolic_hostname, size, 0, (struct sockaddr *)&serv, sizeof(serv));
		retval = select(socket_fd + 1, &mask, NULL, NULL, &timeout);
		if (retval == 0) {
			printf("Server is not responding...exiting...\n");
			free(symbolic_hostname);
			close(socket_fd);
			exit(EXIT_FAILURE);
		}
	}
	free(symbolic_hostname);
	int msg_size = 0;
	socklen_t len = sizeof(serv);
	recvfrom(socket_fd, &msg_size, 4, 0, (struct sockaddr*)&serv, &len);
	printf("Reading <%d> bytes from the server\n", htonl(msg_size));
	char* msg = static_cast<char *>(malloc(sizeof(char) * (htonl(msg_size) + 1)));
	recvfrom(socket_fd, (char *) msg, msg_size, 0, (struct sockaddr*)&serv, &len);
	close(socket_fd);


	free(msg);
	return 0;
}