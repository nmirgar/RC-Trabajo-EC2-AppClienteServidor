#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    // Close sockets
#include <dirent.h>
#include <stdlib.h>

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define NUM_MAX_ARG 10

void list_files(int socket);
void download_file(int socket, char info[]);
void upload_file(int socket, char info[]);
void delete_file(int socket, char info[]);
void rename_file(int socket, char nombreActual[], char nuevoNombre[]);

int main(int argc, char *argv[])
{
    int socket_desc, accepted_socket;
    struct sockaddr_in server_addr; // Direccion del servidor
    char buffer[_BUFFER_SIZE];

    printf("Initializing echo server\n");

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0); // Sock stream --> SOCKET TCP
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
    server_addr.sin_port = htons(ECHO_PORT);

    bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(socket_desc, 10);

    int exit = 0;

    while (!exit)
    {
        printf("Echo server. Waiting for incoming connections from clients.\n");
        accepted_socket = accept(socket_desc, (struct sockaddr *)NULL, NULL);
        printf("Accepted a connection from client\n");

        // First wait for the message of the client
        ssize_t len = recv(accepted_socket, buffer, _BUFFER_SIZE, 0);
        if (len < 0)
        {
            printf("recv failed\n");
        }
        else
        {
            printf("Received data from client: %s\n", buffer); // TODO: If the message is too long --> may print garbage
			//Empezamos a procesar
			//Ejercicio 1: LIST_FILE
			if(strncmp(buffer, "LIST_FILES", strlen("LIST_FILES")) == 0){
				list_files(accepted_socket);
			}
            //Ejercicio 2: DOWNLOAD_FILE
			//./client 127.0.0.1 DOWNLOAD_FILE nombreArchivo
            else if(strncmp(buffer, "DOWNLOAD_FILE", strlen("DOWNLOAD_FILE")) == 0)
            {   
				printf("\nEntramos en download\n");
				printf("\n\nBuffer antes de separar%s\n\n", buffer);				//Recortamos DELETE_FILE y lo guardamos en name0
				char *name0 = strtok(buffer, " ");	
				printf("\n\nBuffer despues de separar%s\n\n", buffer);				//Recortamos DELETE_FILE y lo guardamos en name0
				char *name1 = strtok(NULL, " ");					//Recortamos el nombre del archivo y lo guardamos en name1
				printf("%s\n", name1);								//Imprimimos el nombre
				download_file(accepted_socket, name1);
            }
            //Ejercicio 3: UPOLOAD_FILE
            else if (strcmp(buffer, "UPLOAD_FILE") == 0)
            {
                upload_file(accepted_socket, buffer);
            }
            //Ejercicio 4: DELETE_FILE
            else if (strcmp(buffer, "DELETE_FILE") == 0)
            {
                strtok(buffer, " ");
                char *nombreArchivo = strtok(NULL, " ");
                delete_file(accepted_socket, nombreArchivo);
            }
            //Ejercicio 5: RENAME_FILE
            else if (strcmp(buffer, "RENAME_FILE") == 0)
            {
                //strtok()
                rename_file(accepted_socket, buffer, "nuevoNombre");
            }
            //CLOSE
            else if (strcmp(buffer, "CLOSE") == 0)
            {
                printf("Received CLOSE message --> stopping server\n");
                exit = 1;
            }
			//Ejercicio 6: UNKNOWN_COMMAND
            else
            {
                printf("UNKNOWN COMMAND");
                if(send(accepted_socket, "UNKNOWN_COMMAND", strlen("UNKNOWN_COMMAND"), 0) < 0){
                    printf("Send failed\n");
                }
            }
        }

        close(accepted_socket);
        printf("Accepted connection closed.\n");
        sleep(1);
    }

    printf("Closing binded socket\n");
    close(socket_desc);

    return 0;
}



void list_files(int accepted_socket){

	char file_list[_BUFFER_SIZE] = "\0";
	struct dirent *entry;
	/////////////////////////
	//Procesar
	////////////////////////

	
	//Abrimos el directo actual
	DIR *dir = opendir(".");

    if (dir == NULL) {
        perror("Error al abrir el directorio");
        exit(EXIT_FAILURE); 
    }
	while ((entry = readdir(dir)) != NULL) { //Mientras haya informacion
			// Ignora las entradas "." y "..", estos son los directorios ocultos
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				// Concatena el nombre del archivo al buffer
				strcat(file_list, entry->d_name);
				strcat(file_list, "\n"); 
			}
	}
	//Cerranos directorio
	closedir(dir);
	
	//Enviar info al servidor
	if (send(accepted_socket, file_list, strlen(file_list), 0) < 0)
		{
			printf("Send failed\n");
			//Debemos de cerrar el servidor para que no se quede la conexion abierta
			exit(EXIT_FAILURE);
		}
	printf("Echo sent. Content: %s\n", file_list);
	
}
void download_file(int socket, char info[]){}
void upload_file(int socket, char info[]){}
void delete_file(int socket, char info[]){}
void rename_file(int socket, char nombreActual[], char nuevoNombre[]){}