//
// Created by hector-pc on 12/04/25.
//

#include <pthread.h>
#include<stdio.h>
#include<sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <unistd.h>

#include "sql_recall.h"


pthread_mutex_t ddbb_mutex = PTHREAD_MUTEX_INITIALIZER;



int exist(char * table,char *username)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char query[256];
    sprintf(query, "SELECT username FROM '%s' WHERE username == '%s';",table, username);
    receive_sql receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_users, (void*)&receive, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0)
    {
        return -1;
    }
    return 0;
}



/**
 * @brief
 */
int register_user(char *username)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return 2;
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 2;
    }
    char insert[256];
    //Insertar los primeros parametros en data
    sprintf(insert,
            "INSERT into clients(username) "
            " VALUES('%s');", username);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK)
    {
        if (test != SQLITE_CONSTRAINT)
        {
            fprintf(stderr, "ERROR inserting in primary table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return 2;
        }
        fprintf(stderr, "Error PK duplicated with associated user: %s\n", username);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief
 */
int unregister_user(char *username)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return 2;
    }

    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 2;
    }
    // Nueva consulta preparada
    char delete_query[256];
    sprintf(delete_query, "DELETE FROM clients WHERE username == '%s';", username);
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, delete_query, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error deleting user %s", message_error);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 2;
    }
    if (sqlite3_changes(database) == 0)
    {
        printf("user %s does not exist, no rows deleted\n", username);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    printf("User %s erased correctly\n", username);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief
 */
int connect_client(char * username, __uint32_t port_num, char *ip_addr)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return 2;
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return 3;
    }
    char insert[256], pk[256];

    if (exist("clients" ,username) < 0) {
        return 1;
    }

    sprintf(pk,"%s-%d", ip_addr, port_num);
    sprintf(insert,
            "INSERT into users_connected(port_key, username, ip, port) "
            " VALUES('%s','%s','%s',%d);", pk,username, ip_addr, port_num);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK)
    {
        if (test != SQLITE_CONSTRAINT)
        {
            fprintf(stderr, "ERROR inserting in clients_connected table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return 3;
        }
        fprintf(stderr, "Error PK duplicated with associated user: %s\n", username);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 2;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;

}

/**
 * @brief
 */
int disconnect(char *username)
{






    return 0;
}


/**
 * @brief
 */
int publish(char *name, char *user, char *path, char *description)
{




    return 0;
}

/**
 * @brief
 */
int delete(char *path)
{




    return 0;
}

/**
 * @brief
 */
int list_users(char *username, char users[2048][256], char ips[2048][256], int ports[2048], int *len)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return 3;
    }

    //Se parará la query si el cliente no está registrado o conectado
    if (exist("clients" ,username) < 0) {
        return 1; //No existe
    }
    if (exist("users_connected", username)< 0) {
        return 2; //No conectado
    }

    char query[256];
    sprintf(query, "SELECT username,ip,port FROM users_connected WHERE username != '%s';", username);
    printf("%s\n", query);
    request_query_clients receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_users_query, (void*)&receive, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return 3;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0)
    {
        return 0;  //Si no hay elementos que no sean el propio cliente, no se devuelve fallo, pero se indica que no hay.
    }
    for (int i = 0; i < receive.number; i++) {
        memcpy(users[i], receive.users[i], strlen(receive.users[i]));
        printf("Usuario %d es %s\n", i, users[i]);
        memcpy(ips[i], receive.ips[i], strlen(receive.ips[i]));
        printf("Ip %d es %s\n", i, ips[i]);
        ports[i] = receive.ports[i];
        printf("Port %d es %d\n", i, ports[i]);
    }
    *len = receive.number;
    return 0;
}

/**
 * @brief
 */
int list_content(char *user)
{



    return 0;
}