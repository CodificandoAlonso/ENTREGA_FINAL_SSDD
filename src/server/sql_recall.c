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
int recall_row_data(void *data, int num_columns, char **column_values, char **column_names) {
    /*
  receive_sql *sql = data;
    memcpy(sql->value_1,column_values[0], strlen(column_values[0]));
    sql->value3.x = atoi(column_values[1]);
    sql->value3.y = atoi(column_values[2]);
    sql->empty = 1;

     */
    return 0;
}





/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Con la struct receive_sql, N_values va contando que fila de las recibidas se está devolviendo y va almacenando
 * los valores en un array temporal.
 */
int recall_row_value2_all(void *data, int num_columns, char **column_values, char **column_names) {
      /*
    receive_sql *sql = data;
    char *endptr;
    double value = strtod(column_values[0], &endptr);
    sql->value_2[sql->N_values] = value;
    sql->N_values ++;
    sql->empty = 1;
       */
    return 0;
}
