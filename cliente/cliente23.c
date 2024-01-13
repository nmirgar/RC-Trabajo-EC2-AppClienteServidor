#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <stdlib.h>

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define EXIT "EXIT"
#define LIST_FILES "LIST_FILES"
#define DOWNLOAD_FILE "DOWNLOAD_FILE"


void recibeArchivo(int socket, char* nombreArchivo);
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
    if (argc > 2){
        message = argv[2];

        //LIST FILES
        if(strncmp(message, LIST_FILES, strlen(LIST_FILES)) == 0){
           
            if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
            {
                printf("Send failed\n");
                return 1;
            }
            printf("Message sent. Content: %s\n", argv[2]);
            // Receive a reply from the server
            if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
            {
                printf("Recv failed.\n");
            }
            else
            {
                printf("Reply received. Content: %s\n", server_reply);
            }


        //DOWNLOAD FILE
        }else if(strncmp(message, DOWNLOAD_FILE, strlen(DOWNLOAD_FILE)) == 0){
            //Hemos de crear una variable que almacene argv[3] ya que después del strcat no aparece el valor de argv[3]
            printf("\nEntramos en DOWNLOAD\n");
            char argv3[_BUFFER_SIZE] ;
            strcpy(argv3, argv[3]);
            strcat(message ,";"); //concatenemos el caracter delimitador de palabras
            strcat(message, argv3);
            printf("\nDATOS A ENVIAR %s\n", message);
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
                if(strcmp("ERROR", server_reply) == 0){
                    //No hemos encontrado el nombre del archivo
                    printf("\nNO SE HA ENCONTRADO EL ARCHIVO\n");
                }else{
                    //En este caso, hemos recivido el tamaño, enviamos "ACK"
                    if (send(socket_desc, "ACK", strlen("ACK") + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
                    {
                        printf("Send failed\n");
                        return 1;
                    }
                    //printf("Message sent. Content: %s\n", message);
                    else
                    {
                        recibeArchivo(socket_desc, argv3);
                    }

                    //Tenemos que escuchar el servidor y crear un archivo con los datos del servidor
                }

            }
        }else{
            printf("UNKOWN");
        }
    
    }


    
    if(message == EXIT){
        close(socket_desc);
    }

    return 0;
}

void recibeArchivo(int socket_desc, char * nombreArchivo){
    FILE *f;
    char * server_reply;
    f = fopen(nombreArchivo, "wb");
    if(f == NULL)
    {
        printf("Error al abrir\n");	
    }
    else
    {
        // Receive a reply from the server
        if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
        {
            printf("Recv failed.\n");
        }

        while (strcmp(server_reply, "FIN") != 0)
        {
            //Escribimos
            fwrite(server_reply, 1, _BUFFER_SIZE, f);
            // Receive a reply from the server
            if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
            {
                printf("Recv failed.\n");
            }
        }
        
    }
    fclose(f);
    
}