//
// Created by hector_portatil on 9/04/25.
//
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <netdb.h>
#include <pwd.h>
#include <unistd.h>
#include "message_control.h"
#include "database_control.h"
#define MAX_THREADS 25

//Inicializador global de los fd para la bbdd y la queue del servidor
sqlite3 *database_server = 0;

//Creación de hilos, arrays de hilos y contador de hilos ocupadosc
pthread_t thread_pool[MAX_THREADS];
int sc[MAX_THREADS];
int free_threads_array[MAX_THREADS];

//Inicializador de mutex para la copia local de parámetros, la gestión de la bbdd y variable condicion
//E inicializador de semáforos para gestionar bien la concurrencia
int free_mutex_copy_params_cond = 0;
sem_t available_threads;
pthread_mutex_t mutex_workload;
pthread_mutex_t mutex_copy_params;
pthread_cond_t cond_wait_cpy;


/**
 *@brief Esta función se usa para rellenar el array auxiliar que indica qué hilo está trabajando(1)
 *y cuál está libre(0)
 */
void pad_array() {
    for (int i = 0; i < MAX_THREADS; i++) {
        free_threads_array[i] = 0;
    }
}


char *get_db_username() {
    struct passwd *pw = getpwuid(getuid());
    if (pw) return pw->pw_name;
    return "default";  // fallback si no encuentra usuario
}


/**
 *@brief Esta función se usa para enviar el mensaje de vuelta al cliente, recibe como parametros el socket y la estructura request
 * y con ella envia los datos. es invocada por cada hilo en la funcion process request.
 */
int answer_back(int socket, request *params) {
    return send_message(socket, params);
}

/**
 *@brief Esta función se usa para enviar el mensaje de vuelta al cliente, recibe como parametros el socket y la estructura request
 * y con ella envia los datos. es invocada por cada hilo en la funcion process request.
 */
int answer_back_query(int socket, request_query_clients *params) {
    return send_message_query(socket, params);
}


/**
 *@brief Función que se encarga de "anunciar" que un hilo ha terminado su tarea y pueda volver al ruedo
 */
void end_thread(int thread_id) {
    pthread_mutex_lock(&mutex_workload);
    free_threads_array[thread_id] = 0;
    close(sc[thread_id]);
    sem_post(&available_threads);
    pthread_mutex_unlock(&mutex_workload);
    pthread_exit(NULL);
}


/**
 *@brief Esta es la función que ejecutan los distintos hilos dentro de nuestra pool de hilos
 *Requiere el paso como referencia de una estructura que contenga el índice del socket sobre el que trabaja el hilo
 *Una vez que el codigo realiza la copia local de los datos se encargará de gestionar las distintas llamadas de
 *todo para realizar las gestiones en la base de datos correspondiente
 */
void *process_request(parameters_to_pass *socket) {
    pthread_mutex_lock(&mutex_copy_params);
    int socket_id = socket->identifier;
    char *ip_addr = socket->client_ip;
    free_mutex_copy_params_cond = 1;
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);
    request local_request;
    int message = receive_message(sc[socket_id], &local_request);
    if (message < 0) {
        end_thread(socket_id);
        pthread_exit(0);
    }
    switch (local_request.operation) {
        case 0: //REGISTER
            local_request.answer = register_user(local_request.username);
            if (local_request.answer != 0) {
                printf("Error inserting user:%s\n", local_request.username);
            }
        break;
        case 1: //UNREGISTER
            local_request.answer = unregister_user(local_request.username);
            if (local_request.answer != 0) {
            printf("Error removing user:%s\n", local_request.username);
            }
        break;
        case 2: //CONNECT
            local_request.answer = connect_client(local_request.username, local_request.port, ip_addr);
        if (local_request.answer != 0) {
            printf("Error connecting user:%s\n", local_request.username);
        }
        break;
        case 3: //DISCONNECT
            local_request.answer = disconnect(local_request.username);
        if (local_request.answer != 0) {
            printf("Error disconnecting user:%s\n", local_request.username);
        }
        break;
        case 4: //PUBLISH
             local_request.answer = publish(local_request.username, local_request.path,
                                            local_request.description);
        break;
        case 5: //DELETE
             local_request.answer = delete(local_request.path, local_request.username);
        break;
        case 6: //LIST_USERS
            request_query_clients local_query = {0};
            local_query.answer = list_users(local_request.username, local_query.users, local_query.ips,
                                            local_query.ports, &local_query.number);
            answer_back_query(sc[socket_id], &local_query);
            end_thread(socket_id);
            return NULL;
        case 7: //LIST_CONTENT
            //De momento usaremos el array users de esta estructura
            request_query_clients query_content = {0};
            query_content.answer = list_content(local_request.username,local_request.username2,
                                                query_content.users, &query_content.number);
            //Para que answer back query solo envie un campo
            query_content.content = 1;
            answer_back_query(sc[socket_id], &query_content);
            end_thread(socket_id);
            return NULL;
        default:
            break;
    }

    answer_back(sc[socket_id], &local_request);
    end_thread(socket_id);
    return NULL;
}

/**
 *@brief Función implementada al inicializarse el servidor que creará las tablas de SQL que se encargarán
 *de mantener nuestros datos ordenados. Hemos creado 2 tablas, una "data" que guardará tanto el id(Primary key)
 *como value1 y value3. Value2 como es un array de longitud variable, nos hemos creado una tabla "value2_all"
 *que hereda de data la PK a modo de Foreign Key con UPDATE y DELETE CASCADE(Si se borra la pk de data, se borrarán
 *todas las referencias a ella en "value2-all". La pk de esta tabla será para cada elemento del array, la conversión
 *a entero de la concatenación de la PK con el índice del elemento en el array. Por ejemplo, si para id 3 tengo que
 *insertar el vector {3.44, 2.15, 14.33} tendré 3 filas en esta nueva tabla
 *  PK   FK   VALUE
 *  30   3    3.44
 *  31   3    2.15
 *  32   3    14.33
 */
int create_table(sqlite3 *db) {
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database_server);
        return -4;
    }

    char *new_table =
            "CREATE TABLE IF NOT EXISTS clients("
            " username TEXT PRIMARY KEY"
            ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "ERROR CREATING USERS TABLE %s\n", message_error);
        sqlite3_close(database_server);
        return -4;
    }
    message_error = NULL;
    new_table =
            "CREATE TABLE IF NOT EXISTS users_connected("
            " port_key TEXT PRIMARY KEY,"
            " username TEXT,"
            " ip TEXT,"
            " port INTEGER,"
            "CONSTRAINT fk_origin FOREIGN KEY(username) REFERENCES clients(username)\n ON DELETE CASCADE\n"
            "ON UPDATE CASCADE);";

    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "ERROR CREATING USERS_CONNECTED TABLE %s\n", message_error);
        sqlite3_close(database_server);
        return -4;
    }
    message_error = NULL;
    new_table =
            "CREATE TABLE IF NOT EXISTS publications("
            " path TEXT PRIMARY KEY,"
            " username TEXT,"
            " description TEXT,"
            "CONSTRAINT fk_origin FOREIGN KEY(username) REFERENCES clients(username)\n ON DELETE CASCADE\n"
            "ON UPDATE CASCADE);";

    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "ERROR CREATING PUBLICATIONS TABLE %s\n", message_error);
        sqlite3_close(database_server);
        return -4;
    }
    return 0;
}


/**
 *@brief Función implementada para hacer un cierre seguro del servidor cuando se pulsa CRTL + C
 */
void safe_close() {
    printf("\n-----------------------------------------------\n");
    printf("\nEXIT SIGNAL RECEIVED. CLOSING ALL AND GOODBYE\n");
    printf("-----------------------------------------------\n");
    exit(0);
}


int main(int argc, char **argv) {
    if (argc < 3) {
        perror("Server port not indicated\n");
        exit(-2);
    }
    int port_num = atoi(argv[2]);
    if (port_num <= 0 || port_num > 65535) {
        perror("Bad port\n");
        return -1;
    }

    signal(SIGINT, safe_close);

    //Creando e inicializando la base de datos
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    char db_name[256];
    char *user = get_db_username();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database_server);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        exit(-4);
    }
    //Creo la tabla principal "data" y la subtabla "value2_all"
    if (create_table(database_server) < 0) {
        exit(-4);
    }
    sqlite3_close(database_server);

    //Inicializacion mutex, semaforo y estructuras
    sem_init(&available_threads, 0, MAX_THREADS);
    pthread_mutex_init(&mutex_workload, NULL);
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);
    pad_array();

    //Inicializo lo necesario para los sockets.
    struct sockaddr_in server_addr, client_addr;
    socklen_t size = sizeof(client_addr);
    int sd, val = 0;
    int err;

    //Crear socket 0principal sd
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creating socket\n");
        exit(-2);
    }
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    // bzero al server_addr
    bzero((char *) &server_addr, sizeof(server_addr));



    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t) port_num);

    //bind
    err = bind(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("Error on bind\n");
        return -1;
    }
    //luisten
    err = listen(sd, SOMAXCONN);
    if (err == -1) {
        printf("Error on listen\n");
        return -1;
    }

    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("Error in gethostname");
        exit(-1);
    }
    struct hostent *he = gethostbyname(hostname);
    if (he == NULL) {
        herror("Error in gethostbyname");
        exit(-1);
    }

    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    char *local_ip = inet_ntoa(*addr_list[0]);


    printf("Init Server <%s>:<%d>\n",local_ip, port_num );


    //Vinculacion con servidor RPC



    parameters_to_pass params = {0};
    while (1) {
        sem_wait(&available_threads); // Esperar hasta que haya hilos libres
        //Se realiza el accept en in sc temporal
        int sc_temp = accept(sd, (struct sockaddr *) &client_addr, &size);
        if (sc_temp < 0) {
            sem_post(&available_threads);
            continue;
        }
        //Obtengo la ip del cliente(Necesario para el caso de la peticion publish
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

        //Miro el primer hilo disponible y le mando currar.
        for (int i = 0; i < MAX_THREADS; i++) {
            if (free_threads_array[i] == 0) {
                pthread_mutex_lock(&mutex_workload);
                free_threads_array[i] = 1;
                //Se copia el fd del sc_temp al indice correspondiente
                sc[i] = sc_temp;
                params.identifier = i;
                strcpy(params.client_ip, client_ip); //Ip copiada
                pthread_mutex_unlock(&mutex_workload);
                if (pthread_create(&thread_pool[i], NULL, (void *) process_request, &params) == 0) {
                    pthread_mutex_lock(&mutex_copy_params);
                    while (free_mutex_copy_params_cond == 0)
                        pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                    free_mutex_copy_params_cond = 0;
                    pthread_mutex_unlock(&mutex_copy_params);
                }
                break;
            }
        }
    }
}
