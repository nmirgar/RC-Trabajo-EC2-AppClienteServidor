#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    // Close sockets
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define NUM_MAX_ARG 10

void list_files(int socket);
void download_file(int socket, char *nombreArchivo);
void upload_file(int socket, char info[]);
void delete_file(int socket, char info[]);
void rename_file(int socket, char nombreActual[], char nuevoNombre[]);

void enviarArchivo(int socket, char* nombreArchivo);

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
				strtok(buffer, " ");	//No queremos la primera así que no la guardamos
				char * nombreArchivo = strtok(NULL, " ");	// Guardamos la segunda palabra que es el nombre del archivo				
                download_file(accepted_socket, nombreArchivo);
            }
            //Ejercicio 3: UPOLOAD_FILE
            else if (strncmp(buffer, "UPLOAD_FILE", strlen("UPLOAD_FILE")) == 0)
            {
                //TODO --> IMPLEMENTAR
                upload_file(accepted_socket, buffer);
            }
            //Ejercicio 4: DELETE_FILE
            else if (strncmp(buffer, "DELETE_FILE", strlen("DELETE_FILE")) == 0)
            {
                strtok(buffer, " ");
                char *nombreArchivo = strtok(NULL, " ");
                printf("NOMBRE ARCHIVO %s", nombreArchivo);
                delete_file(accepted_socket, nombreArchivo);
            }
            //Ejercicio 5: RENAME_FILE
            else if (strncmp(buffer, "RENAME_FILE", strlen("RENAME_FILE")) == 0)
            {
                strtok(buffer, " ");
                char * nombreArchivo = strtok(NULL, " ");
                char * nuevoNombre = strtok(NULL, " ");
                rename_file(accepted_socket, nombreArchivo, nuevoNombre);
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
char *nombreArchivosDirectorio(){

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
	if (send(accepted_socket, file_list, strlen(file_list) +1 , 0) < 0)
		{
			printf("Send failed\n");
			//Debemos de cerrar el servidor para que no se quede la conexion abierta
			exit(EXIT_FAILURE);
		}
	printf("Echo sent. Content: %s\n", file_list);
	
}


void download_file(int accepted_socket, char* nombreArchivo){
    int exito = 0; 
    char buffer[_BUFFER_SIZE];
    struct dirent *entry;
	/////////////////////////
	//Procesar
	////////////////////////

	//Abrimos el directo actual
	DIR *dir = opendir(".");
    printf("\n\nAQUI1\n\n");
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        exit(EXIT_FAILURE); 
    }
    printf("\n\nAQUI\n\n");

	while ((entry = readdir(dir)) != NULL) { //Mientras haya informacion
			// Ignora las entradas "." y "..", estos son los directorios ocultos
			if (strcmp(entry->d_name, nombreArchivo) ) {
                exito = 1;   
			}
	}
    printf("\n\nAQUI2\n\n");
    
    if(exito){
        //Enviar longitud al servidor
        printf("\n\nAQUI3\n\n");
        
        if (send(accepted_socket, nombreArchivo, strlen(nombreArchivo) +1 , 0) < 0)
            {
                printf("Send failed\n");
                //Debemos de cerrar el servidor para que no se quede la conexion abierta
                exit(EXIT_FAILURE);
            }
        printf("Echo sent. Content: %s\n", nombreArchivo );

        printf("\n\nAQUI4\n\n");

        //Escuchar si hemos recibido "ACK" entonces enviamos el archivo en binario
        if(recv(accepted_socket, buffer, _BUFFER_SIZE, 0) < 0)
        {
            printf("Recv failed.\n");
        }
        else
        {
            printf("\n\nAQUI5\n\n");

            if(strcmp(buffer, "ACK") == 0)
            {
                enviarArchivo(accepted_socket, nombreArchivo);
            }
        }
    }
    //Si no hemos cambiado el valor de exito es porque no hemos encontrado ese archivo entonces respondemos con ERROR
    else{
        //Enviar ERROR al servidor
        if (send(accepted_socket, "ERROR", strlen("ERROR") +1, 0) < 0)
            {
                printf("Send failed\n");
                //Debemos de cerrar el servidor para que no se quede la conexion abierta
                exit(EXIT_FAILURE);
            }
        printf("Echo sent. Content: ERROR\n");
    }
	//Cerranos directorio
	closedir(dir);

}

//Funcion auxiliar para enviar un archivo
void enviarArchivo(int accepted_socket, char *nombreArchivo){
    FILE *f;
    f = fopen(nombreArchivo, "rb");
    char byteLeidos;
    char buffer[_BUFFER_SIZE];

    printf("Dentro de la funcion enviar archivi\n");
    if(f == NULL){
        printf("Error al abrir el archivo");
    }
    else
    {
        byteLeidos = fread(buffer, 1, _BUFFER_SIZE, f);
        printf("\nEnviando bytes, %s\n", buffer);

        
        do
        {
            printf("\nEnviando bytes, contenido %s\n", buffer);
            if(send(accepted_socket, buffer, strlen(buffer)+1, 0 )< 0){
                printf("\nENVIO FALLIDO\n");
            }else{
                printf("\nEnviadoss bytes\n");
                byteLeidos = fread(buffer, 1, _BUFFER_SIZE, f);
            }

            if(recv(accepted_socket, buffer, _BUFFER_SIZE, 0) < 0){
                printf("Fallo al recibir\n");
            }
            
        }while(!feof);
        if(send(accepted_socket, "FIN", strlen("FIN") +1, 0 )< 0){
                printf("\nENVIO FALLIDO\n");
            }

    }
    fclose(f); 
}

void upload_file(int accepted_socket, char info[]){}
void delete_file(int accepted_socket, char nombreArchivo[]){
    
   
    if(remove(nombreArchivo) == 0) //Se consigue eliminar el archivo
    {
        printf("aqui");
        if(send(accepted_socket, "DELETED", strlen("DELETED") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else //No se consigue eliminar el archivo
    {
        printf("Error: %d\n", errno);
        if(send(accepted_socket, "ERROR", strlen("ERROR") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }
    }
}
void rename_file(int accepted_socket, char nombreActual[], char nuevoNombre[])
{
    if(rename(nombreActual, nuevoNombre ) == 0) //Se consigue cambiar el nombre del archivo
    {
        if(send(accepted_socket, "RENAMED", strlen("RENAMED") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else //No se consigue cambiar el nombre del archivo
    {
        if(send(accepted_socket, "RENAME_ERROR", strlen("RENAME_ERROR") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }
    }
}