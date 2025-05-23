//
// Created by hector-pc on 12/04/25.
//

//
// Created by hectorpc on 28/03/25.
//
#include "message_control.h"
#include "struct.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>





int isBigEndian(void) {
    unsigned int num = 1;
    // Si el primer byte (dirección más baja) es 1, es little endian
    if (*(char *) &num == 1)
        return 0; // little endian
    return 1; //big_endian
}



/**
 * @brief Recibe un paquete de datos a través de un socket.
 *
 * Esta función lee los datos del socket en bloques de tamaño 'size' hasta que se recibe el
 * caracter \0, que endica que la cadena se ha acabado de recibir
 *
 * @param socket Descriptor del socket desde el que se va a leer.
 * @param message Puntero al búfer donde se almacenarán los datos recibidos.
 * @param size Tamaño total del paquete a recibir.
 * @return int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
int receive_characters(int socket, char *message) {

    char *buffer = message; // Buffer para almacenar los datos recibidos

    // Recibimos los datos en partes hasta completar el tamaño total
    int r = 0;
    int i = 0;
    while (i < 255 ) {
        // Leemos del socket y almacenamos en el buffer

        r = read(socket, buffer, 1);
        if (r <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        if (*buffer == '\0') {
            return 0;
        }
        buffer += r; // Movemos el puntero del buffer
        i++;
    }
    return 0; // Retorno exitoso
}

/**
 * @brief Recibe un paquete de datos a través de un socket.
 *
 * Esta función lee los datos del socket en bloques de tamaño 'size' hasta que todo
 * el paquete se ha recibido correctamente. Si ocurre un error en la lectura, se informa.
 *
 * @param socket Descriptor del socket desde el que se va a leer.
 * @param message Puntero al búfer donde se almacenarán los datos recibidos.
 * @param size Tamaño total del paquete a recibir.
 * @return int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
int receive_package(int socket, void *message, int size) {
    int r = 0;
    int left = size;
    void *buffer = message; // Buffer para almacenar los datos recibidos

    // Recibimos los datos en partes hasta completar el tamaño total
    while (left > 0) {
        // Leemos del socket y almacenamos en el buffer
        r = read(socket, buffer, left);
        if (r <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        left -= r; // Restamos la cantidad de bytes leídos
        buffer += r; // Movemos el puntero del buffer
    }
    return 0; // Retorno exitoso
}





/**
 * @brief Recibe un mensaje completo desde un socket y lo deserializa en una estructura 'request'.
 *
 * Esta función maneja la recepción de un mensaje completo, que incluye varios campos de datos como
 * el tipo de mensaje, la clave, los valores asociados, etc. Tras la recepcion individual de los parametros,
 * se deserializan en el formato esperado en la estrucutra. Los nuevos datos se almacenan en la estructura 'message'.
 *
 * @param socket Descriptor del socket desde el que se va a leer el mensaje.
 * @param message Puntero a la estructura 'request' donde se almacenarán los datos recibidos.
 * @return Int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
int receive_message(int socket, request *message) {
    /* Primero se recibe una cadena con la operación a realizar. Esta puede ser:
     * Register Unregister Connect Disconnect Publish Delete List_Users List_Content
     */
    memset(message, 0, sizeof(*message));  // ← limpia toda la estructura

    char receive_char[256];
    int op = receive_characters(socket, receive_char);
    if (op == -1) {
        return -1;
    }
    char date[256];
    op = receive_characters(socket, date);
    if (op == -1) {
        return -1;
    }
    SAFE_CPY(message->datetime, date);
    char username[256];
    op = receive_characters(socket, username);
    if (op == -1) {
        return -1;
    }
    SAFE_CPY(message->username, username);
    printf("OPERATION %s FROM %s\n", receive_char, username);

    //IFS DE CASOS PARA LOS DIFERENTES METODOS DISPONIBLES, DE NO SER NINGUNO SE RETORNA -1
    if (strcmp(receive_char, "REGISTER") == 0) {
        message->operation = 0;  //REGISTER SERÁ 0
        return 0;
    }
    if (strcmp(receive_char, "UNREGISTER") == 0) {
        message->operation = 1;  //UNREGISTER SERÁ 1
        return 0;
    }
    if (strcmp(receive_char, "CONNECT") == 0) {
        message->operation = 2;  //CONNECT SERÁ 2
        op = receive_characters(socket, receive_char);
        if (op == -1) {
            return -1;
        }
        __uint32_t port = atoi(receive_char);
        message->port = port;
        return 0;
    }
    if (strcmp(receive_char, "DISCONNECT") == 0) {
        message->operation = 3; //DISCONNECT SERA 3
        return 0;
    }
    if (strcmp(receive_char, "PUBLISH") == 0) {
        message->operation = 4;  //PUBLISH SERÁ 4
        char path[256];
        op = receive_characters(socket, path);
        if (op == -1) {
            return -1;
        }
        SAFE_CPY(message->path, path);
        char description[256];
        op = receive_characters(socket, description);
        if (op == -1) {
            return -1;
        }
        SAFE_CPY(message->description, description);
        return 0;
    }
    if (strcmp(receive_char, "DELETE") == 0) {
        message->operation = 5;  //DELETE SERÁ 5
        op = receive_characters(socket, username);
        if (op == -1) {
            return -1;
        }
        SAFE_CPY(message->path, username);
        return 0;
    }
    if (strcmp(receive_char, "LIST_USERS") == 0) {
        message->operation = 6;  //LIST_USERS SERÁ 6
        return 0;
    }
    if (strcmp(receive_char, "LIST_CONTENT") == 0) {
        message->operation = 7;  //LIST_CONTENT SERÁ 7
        op = receive_characters(socket, username);
        if (op == -1) {
            return -1;
        }
        SAFE_CPY(message->username2, username);
        return 0;
    }
    return -1;
}



/**
 * @brief Envía un paquete de datos a través de un socket.
 *
 * Esta función envía los datos del paquete al socket en bloques hasta que el paquete completo
 * haya sido enviado correctamente. Si ocurre un error en la escritura, se representa con el codigo de error -1.
 *
 * @param socket Descriptor del socket al que se enviarán los datos.
 * @param message Puntero al paquete de datos a enviar.
 * @param size Tamaño total del paquete a enviar.
 * @return int 0 si el envío fue exitoso, -1 si hubo un error.
 */
int send_package(int socket, void *message, int size) {
    int written = 0;
    int left = size;

    // Enviamos los datos en partes hasta completar el tamaño total
    while (left > 0) {
        written = write(socket, message, left); // Escribimos en el socket
        if (written <= 0) {
            // Si ocurre un error o no se escribió nada
            perror("Error reading from socket");
            return -1;
        }
        left -= written; // Restamos la cantidad de bytes escritos
        message += written; // Movemos el puntero del mensaje
    }
    return 0; // Retorno exitoso
}


/**
 * @brief Envía un mensaje a través de un socket.
 *
 * Esta función serializa los datos de la estructura 'request' y los envía a través del socket,
 * por medio de la funcion definida previamente "send_package()"
 * Convierte los valores al formato de red (big endian) antes de enviarlos.
 * Para la serializacion de los doubles se hace uso de la funcion definida en la practica "host_to_net_double"
 *
 * @param socket Descriptor del socket al que se enviará el mensaje.
 * @param answer Puntero a la estructura 'request' que contiene el mensaje a enviar.
 * @return int 0 si el envío fue exitoso, -1 si hubo un error.
 */
int send_message(int socket, request *answer) {

    __uint8_t *ans = (__uint8_t *) &answer->answer;
    //Segun si la maquina es little o big endian, enviar el byte contenedor de la información
    int sent = 0;
    if (isBigEndian() == 0) { //little endian
        sent = send_package(socket, &ans[0], 1);
    }
    else {   //Big endian
        sent = send_package(socket, &ans[3], 1);
    }
    if (sent<0) {
       perror("Error writing to socket");
        return -1;
    }
    return 0;
}


/**
 * @brief Envía un mensaje a través de un socket.
 *
 * @param socket Descriptor del socket al que se enviará el mensaje.
 * @param answer Puntero a la estructura 'request' que contiene el mensaje a enviar
 * @return int 0 si el envío fue exitoso, -1 si hubo un error.
 */
int send_message_query(int socket, request_query_clients *answer) {


        //Segun si la maquina es little o big endian, enviar el byte contenedor de la información
        __uint8_t *ans = (__uint8_t *) &answer->answer;
        int sent = 0;
        if (isBigEndian() == 0) { //little endian
            sent = send_package(socket, &ans[0], 1);
        }
        else {
            sent = send_package(socket, &ans[3], 1);
        }
        if (sent<0) {
            perror("Error writing to socket");
            return -1;
        }

        if (answer->answer == 0) {
            //Enviar el numero de filas del recall de la base de datos
            int len = snprintf(NULL, 0, "%d", answer->number);
            char *ans2 = malloc(len +1);
            snprintf(ans2, len + 1,"%d", answer->number);
            sent = send_package(socket, ans2, len + 1);
            if (sent<0) {
                perror("Error writing to socket");
                return -1;
            }
            free(ans2);
            //enviar todas esas filas, tanto el user como su ip como su puerto
            for (int i = 0; i < answer->number; i++) {
                sent = send_package(socket, &answer->users[i], strlen(answer->users[i]) + 1);
                if (sent<0) {
                    perror("Error writing to socket");
                    return -1;
                }
                if (answer->content == 0) {
                    sent = send_package(socket, &answer->ips[i], strlen(answer->ips[i]) +1);
                    if (sent<0) {
                        perror("Error writing to socket");
                        return -1;
                    }
                    char cadena[256];
                    sprintf(cadena, "%d", answer->ports[i]);
                    sent = send_package(socket, cadena, strlen(cadena) + 1);
                    if (sent<0) {
                        perror("Error writing to socket");
                        return -1;
                    }
                }
            }
        }
    return 0;
}
