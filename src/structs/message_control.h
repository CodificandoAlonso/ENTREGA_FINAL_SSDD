//
// Created by hectorpc on 28/03/25.
//

#ifndef SOCKET_MESSAGE_H
#define SOCKET_MESSAGE_H
#include "struct.h"




int receive_characters(int socket, char *message);


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
int receive_package(int socket, void *message, int size);


/**
 * @brief Recibe un mensaje completo desde un socket y lo deserializa en una estructura 'request'.
 *
 * Esta función maneja la recepción de un mensaje completo, que incluye varios campos de datos como
 * el tipo de mensaje, la clave, los valores asociados, etc. Tras la recepcion individual de los parametros,
 * se deserializan en el formato esperado en la estrucutra. Los nuevos datos se almacenan en la estructura 'message'.
 *
 * @param socket Descriptor del socket desde el que se va a leer el mensaje.
 * @param message Puntero a la estructura 'request' donde se almacenarán los datos recibidos.
 * @return int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
int receive_message(int socket, request *message);


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
int send_package(int socket, void *message, int size);


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
int send_message(int socket, request *answer);

int send_message_query(int socket, request_query_clients *answer);

#endif //SOCKET_MESSAGE_H
