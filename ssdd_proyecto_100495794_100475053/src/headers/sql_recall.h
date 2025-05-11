#ifndef TREAT_SQL_H
#define TREAT_SQL_H



/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Como esta se realiza sobre users, solo devolverá una única fila con el usuario, si es que existe
 */
int recall_row_users(void *data, int num_columns, char **column_values, char **column_names);

/**
 * @brief
 * Esta función es invocada automaticamente por sqlite3_exec() una vez por cada fila que se devuelve de la query.
 * Con la struct receive_sql, N_values va contando que fila de las recibidas se está devolviendo y va almacenando
 * los valores en un array temporal.
 */
int recall_row_users_query(void *data, int num_columns, char **column_values, char **column_names);

#endif