#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    // Close sockets
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h> //Necesaria para hallar la longitud en bytes del fichero
#include <ctype.h> //isdigit

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define NUM_MAX_ARG 10

void list_files(int socket);
void download_file(int socket, char *nombreArchivo);
void upload_file(int socket, char *nombreArchivo);
void delete_file(int socket, char *nombreArchivo);
void rename_file(int socket, char *nombreActual, char * nuevoNombre);
void enviarArchivo(int socket, char* nombreArchivo);
void recibeArchivo(int socket, char* nombreArchivo);
char *longitud_fichero(char* nombreArchivo);

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
                strtok(buffer, " ");	//No queremos la primera así que no la guardamos
				char * nombreArchivo = strtok(NULL, " ");	// Guardamos la segunda palabra que es el nombre del archivo				
                upload_file(accepted_socket, nombreArchivo);
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


void list_files(int accepted_socket){

	char file_list[_BUFFER_SIZE] = "\0";
	struct dirent *entry;

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

	//Abrimos el directo actual
	DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        exit(EXIT_FAILURE); 
    }

	while ((entry = readdir(dir)) != NULL && !exito) { //Mientras haya informacion
			// Ignora las entradas "." y "..", estos son los directorios ocultos
			if (strcmp(entry->d_name, nombreArchivo) ) {
                exito = 1;   
			}
	}
    
    if(exito){
        //Enviar longitud al servidor
        char *len = longitud_fichero(nombreArchivo);
        printf("\nLONGITUD DEL FICHERO: %s\n", len);
        if (send(accepted_socket, len, strlen(len) +1  , 0) < 0)
            {
                printf("Send failed\n");
                //Debemos de cerrar el servidor para que no se quede la conexion abierta
                exit(EXIT_FAILURE);
            }
        printf("Echo sent. Content: %s\n", nombreArchivo );


        //Escuchar si hemos recibido "ACK" entonces enviamos el archivo en binario
        if(recv(accepted_socket, buffer, _BUFFER_SIZE, 0) < 0)
        {
            printf("Recv failed.\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            if(strcmp(buffer, "ACK") == 0)
            {
                enviarArchivo(accepted_socket, nombreArchivo);
            }
        }
    }
    //Si no hemos cambiado el valor de exito es porque no hemos encontrado ese archivo entonces respondemos con ERROR
    else{
        //Enviar ERROR al cliente
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


void upload_file(int accepted_socket, char * nombreArchivo){
    char buffer [_BUFFER_SIZE];

    //Este primer upload es debido al envío de UOLOAD_FILE nombreArchivo de parte del cliente
    if(send(accepted_socket, "UPLOAD_ACK", strlen("UPLOAD_ACK") + 1, 0) < 0)
    {
        printf("Send failed\n");
        exit(EXIT_FAILURE);
    }
    printf("\nAntes de recibir la longitud\n");
    if(recv(accepted_socket, buffer, _BUFFER_SIZE, 0) < 0)
    {
        printf("Recv failed.\n");
        exit(EXIT_FAILURE);
    }
    //Si recibimos un numero como respuesta esta es la longitud así que la ejecución está siendo correcta
    if(isdigit((int) buffer[0])){
        printf("\nHemos recibido un número\n");
         //Este primer upload es debido al envío de UOLOAD_FILE nombreArchivo de parte del cliente
        if(send(accepted_socket, "UPLOAD_ACK", strlen("UPLOAD_ACK") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }
        recibeArchivo(accepted_socket, nombreArchivo);
        //Si no ha dado ninún fallo en recibeArchivo() enviamos SUCCESS
        if(send(accepted_socket, "SUCCESS", strlen("SUCCESS") + 1, 0) < 0)
        {
            printf("Send failed\n");
            exit(EXIT_FAILURE);
        }else{
            printf("\nSUCCESS\n");
        }
    }else{
        printf("\nNo hemos recibido un número\n");
    }

}
void delete_file(int accepted_socket, char nombreArchivo[]){
    
   
    if(remove(nombreArchivo) == 0) //Se consigue eliminar el archivo
    {
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

char* longitud_fichero(char * nombreArchivo){
    struct stat fichero;
    //Usamos malloc (memory allocator) para assignar espacio en memoria en tiempo de ejecucion
    //así una vez que termine la funcion seguiremos teniendo guardada esa informacion
    char  *len = malloc(_BUFFER_SIZE) ;
                    //Casting de int a char[_BUFFER_SIZE]
    
    if(stat(nombreArchivo, &fichero) == -1){
        printf("Error en el fichero");
        exit(EXIT_FAILURE);
    }
    sprintf(len, "%ld", fichero.st_size);
    return  len;
}


//Funcion auxiliar para enviar un archivo
void enviarArchivo(int accepted_socket, char *nombreArchivo){
    FILE *f;
    f = fopen(nombreArchivo, "rb");
    char byteLeidos;
    char buffer[_BUFFER_SIZE];

    if(f == NULL){
        printf("Error al abrir el archivo");
    }
    else
    {
        byteLeidos = fread(buffer, 1, _BUFFER_SIZE, f);
        

        
        do
        {
            printf("\nEnviando bytes, contenido %s\n", buffer);
            if(send(accepted_socket, buffer, strlen(buffer)+1, 0 )< 0){
                printf("\nENVIO FALLIDO\n");
            }else{
                printf("\nEnviados bytes\n");
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

void recibeArchivo(int socket_desc, char * nombreArchivo){
    FILE *f;
    char server_reply[_BUFFER_SIZE];
    f = fopen(nombreArchivo, "wb");
    if(f == NULL)
    {
        printf("Error al abrir\n");	
    }
    else
    {

        if(recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0){
            printf("Fallo al recibir\n");
        }
        while (strcmp(server_reply, "FIN") != 0)
        {
            fwrite(server_reply, 1, strlen(server_reply) , f);
            if (send(socket_desc, "copiado", strlen("copiado") +1 , 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
            {
                printf("Send failed\n");
            }
            if(recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0){
                printf("Fallo al recibir\n");
            }
        
        
        }
        fclose(f);
    }
}

