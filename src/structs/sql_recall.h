#ifndef TREAT_SQL_H
#define TREAT_SQL_H



/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Como esta se realiza sobre data, solo se realizará un guardado en la estructura receive_sql, y se guarda
 * automáticamente tanto value_1, como value_3
 */
int recall_row_data(void *data, int num_columns, char **column_values, char **column_names);

/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Con la struct receive_sql, N_values va contando que fila de las recibidas se está devolviendo y va almacenando
 * los valores en un array temporal.
 */
int recall_row_value2_all(void *data, int num_columns, char **column_values, char **column_names);

#endif