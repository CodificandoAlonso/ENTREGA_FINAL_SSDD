#!/bin/bash
# Función llamada por la señal SIGINT configurada con trap
cleanup() {
    echo "Interrumpido. Matando procesos..."
    rm -rf /dev/mqueue/client*
    pkill -P $$  # Mata todos los procesos hijos del script
    exit 1
}
# Capturar SIGINT (Ctrl+C)
trap cleanup SIGINT




python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567 &
python3 -u ./src/tests/register_user.py -s localhost -p 3333 -ws 4567


