#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h> //Necesaria para hallar la longitud en bytes del fichero

#define ECHO_PORT 5665
#define _BUFFER_SIZE 2000
#define EXIT "EXIT"
#define LIST_FILES "LIST_FILES"
#define DOWNLOAD_FILE "DOWNLOAD_FILE"
#define UPLOAD_FILE "UPLOAD_FILE"
#define DELETE_FILE "DELETE_FILE"
#define RENAME_FILE "RENAME_FILE"

void list_files(int socket_desc, char* message);
void download_file(int socket_desc, char* message);
void delete_file(int socket_desc, char* message);
void rename_file(int socket_desc, char *message);
char* longitud_fichero(char *filename);
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
           list_files(socket_desc, message);

        //DOWNLOAD FILE
        }else if(strncmp(message, DOWNLOAD_FILE, strlen(DOWNLOAD_FILE)) == 0){
            //Hemos de crear una variable que almacene argv[3] ya que después del strcat no aparece el valor de argv[3]
            char argv3[_BUFFER_SIZE] ;
            strcpy(argv3, argv[3]);
            strcat(message ," "); //concatenemos el caracter delimitador de palabras
            strcat(message, argv3);
            download_file(socket_desc, message );
        }
        //Upload file
        else if(strncmp(message, UPLOAD_FILE, strlen(UPLOAD_FILE)) == 0){
            char argv3[_BUFFER_SIZE] ;
            strcpy(argv3, argv[3]);
            strcat(message ," "); //concatenemos el caracter delimitador de palabras, espacio en blanco
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

            if(strncmp(server_reply, "UPLOAD_ACK", strlen("UPLOAD_ACK")) == 0){
                char * len = longitud_fichero(argv3);
                printf("\nLONGITUD DEL FICHERO: %s\n", len);
                if (send(socket_desc, len, strlen(len) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
                {
                    printf("Send failed\n");
                    return 1;
                }
                

            }else{
                printf("ERROR");
            }
        }
        //DELETE FILE
        else if(strncmp(message, DELETE_FILE, strlen(DELETE_FILE)) == 0){
            char argv3[_BUFFER_SIZE] ;

            strcpy(argv3, argv[3]); //nombre del archivo a borrar
            strcat(message ," "); //concatenemos el caracter delimitador de palabras
            strcat(message, argv3);

            delete_file(socket_desc, message);
        }
        
        //RENAME FILE
        else if(strncmp(message, RENAME_FILE, strlen(RENAME_FILE)) == 0)
        {
            char argv3[_BUFFER_SIZE] ;
            char argv4[_BUFFER_SIZE] ;

            strcpy(argv3, argv[3]); //nombre antiguo del archivo
            strcat(message ," "); //concatenemos el caracter delimitador de palabras
            strcat(message, argv3);

            strcpy(argv4, argv[4]); //nombre nuevo del archivo
            strcat(message, " ");
            strcat(message, argv4);

            rename_file(socket_desc, message);
            
        }

        //COMANDO NO RECONOCIDO
        else{
            printf("UNKOWN");
        }
    
    }

    
    if(message == EXIT){
        close(socket_desc);
    }

    return 0;
}

void list_files(int socket_desc, char* message){
    char server_reply[_BUFFER_SIZE];
    if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
    {
        printf("Send failed\n");
        exit(EXIT_FAILURE);
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

}
void download_file(int socket_desc, char *message){
    char server_reply[_BUFFER_SIZE];
    

    printf("\nDATOS A ENVIAR %s\n", message);
    if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
    {
        printf("Send failed\n");
        exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
            }
            //Recibibimos  DOWNLOAD_FILE nombreArchivo, necesitamos extraer el nombre para pasarselo a la 
            //funcion recibeArchivo(socket_destino, nombreArchivo)
            //Realizamos esto despues de usar message porque strok modifica la cadena orginal
            strtok(message, " ");	//No queremos la primera así que no la guardamos
	        char * nombreArchivo = strtok(NULL, " ");	// Guardamos la segunda palabra que es el nombre del archivo		
            recibeArchivo(socket_desc, nombreArchivo);

            //Tenemos que escuchar el servidor y crear un archivo con los datos del servidor
        }

    }
}
void delete_file(int socket_desc, char* message){
    char server_reply[_BUFFER_SIZE];
    printf("\nDATOS A ENVIAR %s\n", message);
    if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
    {
        printf("Send failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Message sent. Content: %s\n", message);

    if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
    {
        printf("Recv failed.\n");
    }
    else
    {
        printf("Reply received. Content: %s\n", server_reply);
    }
}

void rename_file(int socket_desc, char* message){
    
    char server_reply[_BUFFER_SIZE];
    printf("\nDATOS A ENVIAR %s\n", message);

    if (send(socket_desc, message, strlen(message) + 1, 0) < 0) // Importante el +1 para enviar el finalizador de cadena!!
    {
        printf("Send failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Message sent. Content: %s\n", message);

    if (recv(socket_desc, server_reply, _BUFFER_SIZE, 0) < 0)
    {
        printf("Recv failed.\n");
    }
    else
    {
        printf("Reply received. Content: %s\n", server_reply);
    }
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