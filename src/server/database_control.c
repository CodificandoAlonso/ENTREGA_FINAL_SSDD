//
// Created by hector-pc on 12/04/25.
//

#include <database_control.h>
#include <pthread.h>
#include <pwd.h>
#include<stdio.h>
#include<sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <unistd.h>
#include "sql_recall.h"

/**
 *@brief Funcion encargada de busar el hostname que realiza la ejecución del codigo. Necesario para la ddbb
 */
char *get_username_db() {
    struct passwd *pw = getpwuid(getuid());
    if (pw) return pw->pw_name;
    return "default"; // fallback si no encuentra usuario
}


pthread_mutex_t ddbb_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 *@brief Funcion encargada de busar en una table genérica un valor específico
 */
int exist(char *table, char *username, char *type) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char query[256];
    sprintf(query, "SELECT %s FROM %s WHERE %s == '%s';", type, table, type, username);
    receive_sql receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_users, (void *) &receive, NULL) != SQLITE_OK) {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0) {
        return -1;
    }
    return 0;
}

/**
 *@brief Funcion encargada de eliminar una query pasada como parámetro
 */
int delete_generic(char *query) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return -1;
    }
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error deleting generic %s", message_error);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    if (sqlite3_changes(database) == 0)
    {
        pthread_mutex_unlock(&ddbb_mutex);
        sqlite3_close(database);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}


/**
 * @brief Función encargada de registrar un usuario
 */
int register_user(char *username) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 2; //error generico
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 2; //Error generico
    }
    char insert[256];
    //Query de insercion
    sprintf(insert,
            "INSERT into clients(username) "
            " VALUES('%s');", username);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK) {
        if (test != SQLITE_CONSTRAINT) {
            fprintf(stderr, "ERROR inserting in primary table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return 2; //error generico
        }
        fprintf(stderr, "Error PK duplicated with associated user: %s\n", username);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 1; //Already registered
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Función encargada de borrar de la tabla clients el usuario solicitado. Realiza un delete cascada sobre
 * el resto de tablas con referencias a ese usuario
 */
int unregister_user(char *username) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);

    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 2; //Error generico
    }
    if (exist("clients", username, "username") < 0) {
        return 1; //No registrado
    }
    if (exist("users_connected", username, "username") == 0) {
        return 3; //No registrado
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 2; //error generico
    }
    // Nueva consulta preparada
    char delete_query[256];
    sprintf(delete_query, "DELETE FROM clients WHERE username == '%s';", username);
    if (delete_generic(delete_query) < 0) {
        return 2; //Error generico
    }
    return 0;
}

/**
 * @brief Funcion encargada de insertar en la tabla de clientes conectados el cliente solicitado
 */
int connect_client(char *username, __uint32_t port_num, char *ip_addr) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 3; //error generico
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 3; //error generico
    }
    char insert[1024], pk[256];

    //Verificar que esté registrado
    if (exist("clients", username, "username") < 0) {
        return 1; //no registrado
    }

    //Verificar que no esté conectado
    if (exist("users_connected", username, "username") == 0) {
        return 2; //Ya conectado
    }

    sprintf(pk, "%s-%d", ip_addr, port_num);
    sprintf(insert,
            "INSERT into users_connected(port_key, username, ip, port) "
            " VALUES('%s','%s','%s',%d);", pk, username, ip_addr, port_num);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK) {
        if (test != SQLITE_CONSTRAINT) {
            fprintf(stderr, "ERROR inserting in clients_connected table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return 3; //Error genérico
        }
        fprintf(stderr, "Error PK duplicated with associated user: %s\n", username);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 2; //Error client connected
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Función encargada de eliminar de la tabla de clientes conectados al usuario solicitado
 */
int disconnect(char *username) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 2; //Error generico
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 3;
    }
    char query[256];
    if (exist("clients", username, "username") < 0) {
        return 1; //Cliente no registrado
    }
    if (exist("users_connected", username, "username") < 0) {
        return 2; //Cliente no conectado
    }
    sprintf(query, "Delete from users_connected WHERE username =='%s';", username);
    if (delete_generic(query) < 0) {
        return 3; //error genérico
    }
    sqlite3_close(database);
    return 0; //Exito
}


/**
 * @brief Funcion encargada de guardar en la tabla publications tanto el filename, la description como el usuario
 * que realiza la operación
 */
int publish(char *user, char *path, char *description) {
    sqlite3 *database;
    char db_name[256];
    char *user_ddbb = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user_ddbb);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 2;
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 3;
    }
    if (exist("clients", user, "username") < 0) {
        return 1; //no registrado
    }
    if (exist("users_connected", user, "username") < 0) {
        return 2; //no conectado
    }
    char *insert = malloc(1024);
    sprintf(insert,
            "INSERT into publications(path, username, description) "
            " VALUES('%s','%s','%s');", path, user, description);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK) {
        if (test != SQLITE_CONSTRAINT) {
            fprintf(stderr, "ERROR inserting in publications table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            free(insert);
            pthread_mutex_unlock(&ddbb_mutex);
            return 4; //error generico
        }
        fprintf(stderr, "Error PK duplicated with associated filepath: %s\n", path);
        sqlite3_close(database);
        free(insert);
        pthread_mutex_unlock(&ddbb_mutex);
        return 3; //error contenido ya publicado
    }
    free(insert);
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Funcion encargada de eliminar el contenido especificado
 */
int delete(char *path, char *username) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 4; //Error generico
    }
    if (exist("clients", username, "username") < 0) {
        return 1; //No registrado
    }
    if (exist("users_connected", username, "username") < 0) {
        return 2; //no conectado
    }
    if (exist("publications", path, "path") < 0) {
        return 3; //no publicado el contenido
    }
    char query[512];
    sprintf(query, "Delete from publications WHERE path = '%s' AND username = '%s';", path, username);
    if (delete_generic(query) < 0) {
        return 4; //error generico
    }
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Funcion encargada de recoger todas las filas de la tabla users connected menos la fila del usuario
 * solicitante
 */
int list_users(char *username, char users[2048][256], char ips[2048][256], int ports[2048], int *len) {
    sqlite3 *database;
    char db_name[256];
    char *user = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 3; //error generico
    }

    //Se parará la query si el cliente no está registrado o conectado
    if (exist("clients", username, "username") < 0) {
        return 1; //No existe
    }
    if (exist("users_connected", username, "username") < 0) {
        return 2; //No conectado
    }

    char query[256];
    sprintf(query, "SELECT username,ip,port FROM users_connected WHERE username != '%s';", username);
    request_query_clients receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_users_query, (void *) &receive, NULL) != SQLITE_OK) {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 3; //error generico
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0) {
        return 0; //Si no hay elementos que no sean el propio cliente, no se devuelve fallo, pero se indica que no hay.
    }
    for (int i = 0; i < receive.number; i++) {
        memcpy(users[i], receive.users[i], strlen(receive.users[i]));
        memcpy(ips[i], receive.ips[i], strlen(receive.ips[i]));
        ports[i] = receive.ports[i];
    }
    *len = receive.number;
    return 0;
}

/**
 * @brief Funcion encargada de sacar todas las filas de publications cuyo usuario poseedor sea el solicitado
 */
int list_content(char *user, char *user_content, char users[2048][256], int *len) {
    sqlite3 *database;
    char db_name[256];
    char *user_ddbb = get_username_db();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user_ddbb);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return 4; //error generico
    }

    //Se parará la query si el cliente no está registrado o conectado
    if (exist("clients", user, "username") < 0) {
        return 1; //No existe el user solicitante
    }
    if (exist("users_connected", user, "username") < 0) {
        return 2; //No conectado el user solicitante
    }

    if (exist("clients", user_content, "username") < 0) {
        return 3; //No existe el user a buscar
    }

    char query[256];
    sprintf(query, "SELECT path FROM publications WHERE username == '%s';", user_content);
    request_query_clients receive = {0};
    receive.content = 1;
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_users_query, &receive, NULL) != SQLITE_OK) {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 4; //error generico
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0) {
        return 0; //Si no hay elementos no es fallo
    }
    for (int i = 0; i < receive.number; i++) {
        memcpy(users[i], receive.users[i], strlen(receive.users[i]));
    }
    *len = receive.number;
    return 0;
}
