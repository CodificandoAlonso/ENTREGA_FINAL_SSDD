//
// Created by hector-pc on 12/04/25.
//
//
// Created by hector-pc on 12/04/25.
//
#include "claves.h"

#include <pthread.h>
#include<stdio.h>
#include<sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <unistd.h>

#include "treat_sql.h"


pthread_mutex_t ddbb_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief Esta llamada permite inicializar el servicio de elementos clave-valor1-valor2-valor3.
 * Mediante este servicio se destruyen todas las tuplas que estuvieran almacenadas previamente.
 *
 * @return int La función devuelve 0 en caso de éxito y -1 en caso de error.
 * @retval 0 en caso de exito.
 * @retval -1 en caso de error.
 */
int destroy()
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char* message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return -4;
    }

    char* delete_data_table = "DELETE from data;";
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, delete_data_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR deleting tables %s\n", sqlite3_errmsg(database));
        pthread_mutex_unlock(&ddbb_mutex);
        sqlite3_close(database);
        return -1;
    }
    // Verificar si realmente se eliminó una fila
    if (sqlite3_changes(database) == 0)
    {
        printf("Database is already clean, no rows deleted\n");
        pthread_mutex_unlock(&ddbb_mutex);
        sqlite3_close(database);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio inserta el elemento <key, value1, value2, value3>.
 * El vector correspondiente al valor 2 vendrá dado por la dimensión del vector (N_Value2) y
 * el vector en si (V_value2).
 * El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * Se considera error, intentar insertar una clave que ya existe previamente o
 * que el valor N_value2 esté fuera de rango. En este caso se devolverá -1 y no se insertará.
 * También se considerará error cualquier error en las comunicaciones.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32].
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 si se insertó con éxito.
 * @retval -1 en caso de error.
 */
int set_value(int key, char* value1, int N_value2, double* V_value2, struct Coord value3)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return -4;
    }
    value1[strcspn(value1, "\r\n")] = 0;
    char insert[256];
    char local_value1[256];
    memcpy(local_value1, value1, strlen(value1));

    //Insertar los primeros parametros en data
    sprintf(insert,
            "INSERT into data(data_key, value1,x,y) "
            " VALUES(%d, '%s', %d ,%d);", key, value1, value3.x, value3.y);
    int test;
    pthread_mutex_lock(&ddbb_mutex);
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK)
    {
        if (test != SQLITE_CONSTRAINT)
        {
            fprintf(stderr, "ERROR inserting in primary table %s\n", sqlite3_errmsg(database));
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return -1;
        }
        fprintf(stderr, "Error PK duplicated with associated key: %d\n", key);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (N_value2 > 32)
    {
        fprintf(stderr, "Too many arguments in value2\n");
        sqlite3_close(database);
        return -1;
    }
    char primary_key[20];

    for (int i = 0; i < N_value2; i++)
    {
        sprintf(primary_key, "%d%d", key, i);
        sprintf(insert,
                "INSERT into value2_all(id, data_key_fk, value) "
                " VALUES(%s, %d, %f);", primary_key, key, V_value2[i]);
        pthread_mutex_lock(&ddbb_mutex);
        if ((test = sqlite3_exec(database, insert, NULL, NULL, &message_error)) != SQLITE_OK)
        {
            if (test != SQLITE_CONSTRAINT)
            {
                fprintf(stderr, "ERROR inserting in secondary table.\n");
                pthread_mutex_unlock(&ddbb_mutex);
                sqlite3_close(database);
                return -1;
            }
            fprintf(stderr, "Error PK duplicated in secondary table.\n");
            sqlite3_close(database);
            pthread_mutex_unlock(&ddbb_mutex);
            return -1;
        }
        pthread_mutex_unlock(&ddbb_mutex);
    }

    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite obtener los valores asociados a la clave key.
 * La cadena de caracteres asociada se devuelve en value1.
 * En N_Value2 se devuelve la dimensión del vector asociado al valor 2 y en V_value2 las componentes del vector.
 * Tanto value1 como V_value2 tienen que tener espacio reservado para poder almacenar el máximo número
 * de elementos posibles (256 en el caso de la cadena de caracteres y 32 en el caso del vector de doubles).
 * La función devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo,
 * si no existe un elemento con dicha clave o si se produce un error de comunicaciones.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32] por referencia.
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord por referencia.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 en caso de éxito.
 * @retval -1 en caso de error.
 */
int get_value(int key, char* value1, int* N_value2, double* V_value2, struct Coord* value3)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }

    char query[256];
    sprintf(query, "SELECT value1, x, y FROM data WHERE data_key == %d;", key);
    receive_sql receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_data, (void*)&receive, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    //controlador a modo de "flag" por si no se devuelve ninguna fila
    if (receive.empty == 0)
    {
        sqlite3_close(database);
        return -1;
    }
    receive.empty = 0;
    sprintf(query, "SELECT value FROM value2_all WHERE data_key_fk == %d;", key);
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_value2_all, (void*)&receive, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0)
    {
        sqlite3_close(database);
        return 0; //Este caso seria en el que no hay elementos de value2, es decir, se ha insertado un vector vacio
        //No está mal, de ahi que se devuelva 0, simplemente no se realiza la copia
    }
    memcpy(value1, receive.value_1, sizeof(receive.value_1) * sizeof(char));
    for (int i = 0; i < receive.N_values; i++)
    {
        V_value2[i] = receive.value_2[i];
    }
    *N_value2 = receive.N_values;
    value3->x = receive.value3.x;
    value3->y = receive.value3.y;
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite modificar los valores asociados a la clave key.
 * La función devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo,
 * si no existe un elemento con dicha clave o si se produce un error en las comunicaciones.
 * También se devolverá -1 si el valor N_value2 está fuera de rango.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32].
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 si se modificó con éxito.
 * @retval -1 en caso de error.
 */
int modify_value(int key, char* value1, int N_value2, double* V_value2, struct Coord value3)
{
    if (delete_key(key) < 0)
    {
        return -1;
    }
    if (set_value(key, value1, N_value2, V_value2, value3) < 0)
    {
        return -1;
    }
    return 0;
}


/**
 * @brief Este servicio permite borrar el elemento cuya clave es key.
 * La función devuelve 0 en caso de éxito y -1 en caso de error.
 * En caso de que la clave no exista también se devuelve -1.
 *
 * @param key clave.
 * @return int La función devuelve 0 en caso de éxito y -1 en caso de error.
 * @retval 0 en caso de éxito.
 * @retval -1 en caso de error.
 */
int delete_key(int key)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }

    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return -4;
    }
    // Nueva consulta preparada
    char delete_query[256];
    sprintf(delete_query, "DELETE FROM data WHERE data_key == %d;", key);
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, delete_query, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error deleting key %s", message_error);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    if (sqlite3_changes(database) == 0)
    {
        printf("Key %d does not exist, no rows deleted\n", key);
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    printf("Key %d erased correctly\n", key);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite determinar si existe un elemento con clave key.
 * La función devuelve 1 en caso de que exista y 0 en caso de que no exista.
 * En caso de error se devuelve -1. Un error puede ocurrir en este caso por un problema en las comunicaciones.
 *
 * @param key clave.
 * @return int La función devuelve 1 en caso de que exista y 0 en caso de que no exista. En caso de error se devuelve -1.
 * @retval 1 en caso de que exista.
 * @retval 0 en caso de que no exista.
 * @retval -1 en caso de error.
 */
int exist(int key)
{
    sqlite3* database;
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char query[256];
    sprintf(query, "SELECT value1, x, y FROM data WHERE data_key == %d;", key);
    receive_sql receive = {0};
    pthread_mutex_lock(&ddbb_mutex);
    if (sqlite3_exec(database, query, recall_row_data, (void*)&receive, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR executing query\n");
        sqlite3_close(database);
        pthread_mutex_unlock(&ddbb_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ddbb_mutex);
    if (receive.empty == 0)
    {
        return 0;
    }
    return 1;
}
