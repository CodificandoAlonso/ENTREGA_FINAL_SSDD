#!/bin/bash

USERNAME=$(whoami)

DB_PATH="/tmp/database-$USERNAME.db"

if [ -f "$DB_PATH" ]; then
    echo "Eliminando base de datos de usuario $USERNAME..."
    rm -f "$DB_PATH"

    if [ $? -eq 0 ]; then
        echo "Base de datos eliminada: $DB_PATH"
    else
        echo "Error al eliminar la base de datos" >&2
        exit 1
    fi
else
    echo "No se encontró base de datos para el usuario $USERNAME"
    exit 0
fi