//
// Created by hector-pc on 12/04/25.
//
#include "struct.h"
#include "sql_recall.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>


/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Como esta se realiza sobre data, solo se realizará un guardado en la estructura receive_sql, y se guarda
 * automáticamente tanto value_1, como value_3
 */
int recall_row_users(void *data, int num_columns, char **column_values, char **column_names) {
    (void)num_columns;
    (void)column_names;
    receive_sql *sql = data;
    memcpy(sql->user,column_values[0], strlen(column_values[0]));
    sql->empty = 1;

    return 0;
}





/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Con la struct receive_sql, number va contando que fila de las recibidas se está devolviendo y va almacenando
 * los valores en un array temporal.
 */
int recall_row_users_query(void *data, int num_columns, char **column_values, char **column_names) {
    (void)num_columns;
    (void)column_names;
    request_query_clients *sql = data;
    if (sql->content == 0)
    {
        memcpy(sql->users[sql->number],column_values[0], strlen(column_values[0]));
        memcpy(sql->ips[sql->number],column_values[1], strlen(column_values[1]));
        sql->ports[sql->number] = atoi(column_values[2]);
        sql->number ++;
        sql->empty = 1;
    }
    else
    {
        memcpy(sql->users[sql->number],column_values[0], strlen(column_values[0]));
        sql->number ++;
        sql->empty = 1;
    }

    return 0;
}
