#ifndef STRUCT_H
#define STRUCT_H
#include <bits/types.h>
#include <netinet/in.h>


/**
 * @brief
 *Estructura que se usará para la cola de mensajes
 */
typedef struct request {
    int operation;
    char username[256];
    int ip;
    __uint32_t port;
    char path[256];
    char description[256];
    int answer;
}request;



/**
 * @brief
 *Estructura que se usará para pasar los parámetros a los hilos
 */
typedef struct parameters_to_pass_threads
{
    int identifier;
    char client_ip[INET_ADDRSTRLEN];
} parameters_to_pass;


typedef struct receive_sql {

}receive_sql;






#endif