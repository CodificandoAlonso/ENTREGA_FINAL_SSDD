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
    char client_ip[INET_ADDRSTRLEN];
    __uint32_t port;
    char path[256];
    char description[256];
    int answer;
}request;



typedef struct request_query_clients {
    char users[2048][256];
    char ips[2048][256];
    int ports[2048];
    int number;
    int empty;
    int answer;
}request_query_clients;

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
    int empty;
    char user[256];
    char users[2048][256];
    char ips[2048][256];
    int ports[2048];
}receive_sql;






#endif