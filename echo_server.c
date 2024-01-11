#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include <unistd.h> // Close sockets

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000

int main(int argc , char *argv[])
{
	int socket_desc, accepted_socket;
	struct sockaddr_in server_addr; // Direccion del servidor
	char buffer[_BUFFER_SIZE];
	
	printf("Initializing echo server\n");

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0); // Sock stream --> SOCKET TCP
	if (socket_desc < 0)
	{
		printf("Could not create socket\n");
		return 0;
	}
		
	printf("Socket created\n");

    char *address = "0.0.0.0"; // Accept connections from the whole Internet
    if (argc > 1)
        address = argv[1];
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( ECHO_PORT );

    bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)); 

    listen(socket_desc, 10); 

	int exit = 0;

    while(!exit)
    {
		printf("Echo server. Waiting for incoming connections from clients.\n");
        accepted_socket = accept(socket_desc, (struct sockaddr*)NULL, NULL); 
		printf("Accepted a connection from client\n");

        // First wait for the message of the client
		ssize_t len = recv(accepted_socket, buffer , _BUFFER_SIZE , 0);
		if( len < 0)
		{
			printf("recv failed\n");
		} else {
			printf("Received data from client: %s\n", buffer); // TODO: If the message is too long --> may print garbage
			
			if( send(accepted_socket , buffer , len, 0) < 0) 
			{
				printf("Send failed\n");
				return 1;
			}
			printf("Echo sent. Content: %s\n", buffer);

			if (strcmp(buffer, "CLOSE") == 0) {
				printf("Received CLOSE message --> stopping server\n");
				exit = 1;
			}
		}
		
		close(accepted_socket);
		printf("Accepted connection closed.\n");
		sleep(1);
	}

	printf ("Closing binded socket\n");
	close(socket_desc);

	return 0;
}
