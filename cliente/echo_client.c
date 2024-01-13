#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define LIST_FILE "LIST_FILE"
#define EXIT "EXIT"
int main(int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server;
    char *message, server_reply[_BUFFER_SIZE];

    printf("Initializing socket\n");

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    char *address = "127.0.0.1";
    if (argc > 1)
        address = argv[1]; // Si existe, el primer argumento es la IP
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_PORT);

    printf("Trying to connect to address: %s\n", address);

    // Connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connect error\n");
        return 1;
    }

    printf("Connected Successfully\n");

    // Send some data
    message = "ECHO";
    if (argc > 2)
        message = argv[2]; // Si existe, el segundo argumento es el mensaje


    
    if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
    {
        printf("Send failed\n");
        return 1;
    }
    printf("Message sent. Content: %s\n", message);

    // Receive a reply from the server
    if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
    {
        printf("Recv failed.\n");
    }
    else
    {
        printf("Reply received. Content: %s\n", server_reply);
    }
    
    if(message == EXIT){
        close(socket_desc);
    }

    return 0;
}