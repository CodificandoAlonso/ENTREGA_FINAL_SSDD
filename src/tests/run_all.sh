#!/usr/bin/env bash
# ------------------------------------------------------------
#  B A T E R Í A   D E   P R U E B A S   P A R A  client.py
# ------------------------------------------------------------
RED="\033[31m"; GREEN="\033[32m"; BLUE="\033[34m"; RESET="\033[0m"

SERVER="localhost"
PORT=3333
WSPORT=4567
PY=python3          # cámbialo si usas otro intérprete

mkdir -p temp

echo -e "${BLUE}\nPRUEBAS  |  Sistemas Distribuidos – Cliente P2P${RESET}\n"

# ------------------------------------------------------------
#  CASOS “NORMALES”: un solo proceso cliente
# ------------------------------------------------------------
run_test() {
    local num="$1"; shift
    local name="$1"; shift
    local cmd_file="temp/${num}_${name}.cmd"
    local raw_out="temp/${num}_${name}.raw"
    local out_file="temp/${num}_${name}.out"   # sin prompts
    local exp_file="expected/${num}_${name}.exp"

    #
    printf '%s\nQUIT\n' "$*" > "$cmd_file"

    #
    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$cmd_file" > "$raw_out" 2>/dev/null

    #
    sed 's/^c>  *//' "$raw_out" > "$out_file"

    if [[ "$name" == list_users_* ]]; then
            strip_port < "$out_file" > "${out_file}.nop"
            strip_port < "$exp_file" > "temp/exp_${name}.nop"
            out_file="${out_file}.nop"
            exp_file="temp/exp_${name}.nop"
        fi


    #Compara
    if diff -qZ "$out_file" "$exp_file" >/dev/null; then
        echo -e "Test $num ($name): ${GREEN}SUCCESS${RESET}"
    else
        echo -e "Test $num ($name): ${RED}FAIL${RESET}"
        diff -uZ "$exp_file" "$out_file" | sed 's/^/    /'
    fi
}

# ------------------------------------------------------------
#  CASOS “PARALELOS”: dos procesos cliente simultáneos
# ------------------------------------------------------------
run_parallel_test() {
    local num="$1";          shift
    local name="$1";         shift
    local cmdA="$1";         shift   # comandos cliente A
    local cmdB="$1";         shift   # comandos cliente B
    local exp_file="expected/${num}_${name}.exp"
    local out_file="temp/${num}_${name}.out"

    # 1. Ficheros de comandos
    local fileA="temp/${num}_${name}_A.cmd"
    local fileB="temp/${num}_${name}_B.cmd"
    printf '%s\n' "$cmdA"           > "$fileA"   # SIN QUIT: A debe seguir vivo
    printf '%s\nQUIT\n' "$cmdB"     > "$fileB"

    # 2. Lanza cliente A en segundo plano
    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileA" > /dev/null 2>/dev/null &  pidA=$!

    sleep 0.3                              # le damos tiempo a conectar

    # 3. Lanza cliente B y captura su salida
    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileB" > "$out_file" 2>/dev/null

    # 4. Detiene cliente A
    kill "$pidA" 2>/dev/null
    wait "$pidA" 2>/dev/null

    # 5. Limpia prompts y compara
    sed 's/^c>  *//' "$out_file" > "${out_file}.noprompt"

    if diff -qZ "${out_file}.noprompt" "$exp_file" >/dev/null; then
        echo -e "Test $num ($name): ${GREEN}SUCCESS${RESET}"
    else
        echo -e "Test $num ($name): ${RED}FAIL${RESET}"
        diff -uZ "$exp_file" "${out_file}.noprompt" | sed 's/^/    /'
    fi
}

run_getfile_parallel() {
    local num="$1";  shift
    local name="$1"; shift
    local src="$1";  shift        # archivo original
    local dst="$1";  shift        # nombre destino
    local cmdA="$1"; shift        # comandos cliente A  (mantiene conexión)
    local cmdB="$1"; shift        # comandos cliente B  (GET_FILE)
    local exp_file="expected/${num}_${name}.exp"
    local out_file="temp/${num}_${name}.out"

    # --- prepara scripts de entrada ---
    local fileA="temp/${num}_${name}_A.cmd"
    local fileB="temp/${num}_${name}_B.cmd"
    printf '%s\n'  "$cmdA" > "$fileA"            # SIN QUIT → se queda conectado
    printf '%s\nQUIT\n' "$cmdB" > "$fileB"

    # --- lanza cliente A (benito) de fondo ---
    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileA" > /dev/null 2>/dev/null & pidA=$!

    sleep 0.3    # da tiempo a que A complete el CONNECT

    # --- lanza cliente B y captura salida ---
    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileB" > "$out_file" 2>/dev/null

    # --- detiene A ---
    kill "$pidA" 2>/dev/null
    wait "$pidA" 2>/dev/null

    # --- limpia prompts en la salida de B ---
    sed 's/^c>  *//' "$out_file" > "${out_file}.noprompt"

    # --- verifica salida textual ---
    if ! diff -qZ "${out_file}.noprompt" "$exp_file" >/dev/null; then
        echo -e "Test $num ($name): ${RED}FAIL (salida)${RESET}"
        diff -uZ "$exp_file" "${out_file}.noprompt" | sed 's/^/    /'
        rm -f "$dst"
        return
    fi

    # --- compara archivos ---
    if cmp -s "$src" "$dst"; then
        echo -e "Test $num ($name): ${GREEN}SUCCESS${RESET}"
    else
        echo -e "Test $num ($name): ${RED}FAIL (fichero distinto)${RESET}"
        echo "    Archivos difieren: $src  vs  $dst"
    fi
    rm -f "$dst"        #DESCOMENTAR SI SE QUIERE VER EL ARCHIVO
                        #SI SE COMENTA, EL DE FALLO DE FILE NOT EXIST QUIZAS FALLE
}

strip_port() {
    sed -E 's/[[:space:]][0-9]{1,5}$//'
}

run_getfile_fail() {
    local num="$1";  shift
    local name="$1"; shift
    local dst="$1";  shift
    local cmdA="$1"; shift        # comandos cliente A
    local cmdB="$1"; shift        # comandos cliente B
    local exp_file="expected/${num}_${name}.exp"
    local out_file="temp/${num}_${name}.out"

    # scripts de entrada
    local fileA="temp/${num}_${name}_A.cmd"
    local fileB="temp/${num}_${name}_B.cmd"
    printf '%s\n'  "$cmdA"        > "$fileA"
    printf '%s\nQUIT\n' "$cmdB"   > "$fileB"

    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileA" > /dev/null 2>/dev/null & pidA=$!
    sleep 0.3

    $PY ../client/client.py -s "$SERVER" -p "$PORT" -ws "$WSPORT" \
        < "$fileB" > "$out_file" 2>/dev/null

    kill "$pidA" 2>/dev/null
    wait "$pidA" 2>/dev/null

    sed 's/^c>  *//' "$out_file" > "${out_file}.nop"

    if diff -qZ "${out_file}.nop" "$exp_file" >/dev/null; then
        if [[ ! -e $dst ]]; then
            echo -e "Test $num ($name): ${GREEN}SUCCESS${RESET}"
        else
            echo -e "Test $num ($name): ${RED}FAIL (se creó $dst)${RESET}"
        fi
    else
        echo -e "Test $num ($name): ${RED}FAIL (salida)${RESET}"
        diff -uZ "$exp_file" "${out_file}.nop" | sed 's/^/    /'
    fi
    rm -f "$dst"
}




# ---------------------- CASOS ----------------------

#PUBLISH FAIL, USER DOES NOT EXIST NUNCA SALDRÁ EN ESTE CODIGO, PORQUE SOLO ENVIA USER SI ESTÁ CONECTADO, O SI APARECE
#CONECTADO EN EL CLIENTE. LO MISMO CON DELETE FAIL, USER DOES NOT EXIST, Y CON LIST_USERS Y LIST_CONTENT

run_test 01 register_ok              "REGISTER pepe"
run_test 02 register_dup             $'REGISTER david\nREGISTER david'
run_test 03 unregister_ok            "UNREGISTER pepe"
run_test 04 unregister_dne           "UNREGISTER fantasma"
run_test 05 connect_ok               $'REGISTER benito\nCONNECT benito'
run_test 06 connect_dup              $'CONNECT benito\nCONNECT benito'
run_test 07 connect_dne              "CONNECT doesnotexist"
# Caso paralelo: “usuario ya conectado” debería fallar con código 2
run_parallel_test 08 connect_dup_err \
    $'REGISTER pedro\nCONNECT pedro' \
    'CONNECT pedro'
run_test 09 disconnect_ok            $'CONNECT benito\nDISCONNECT benito'
run_test 10 disconnect_not_conn      "DISCONNECT benito"
#CODIGO DE ERROR NO ESPECIFICADO POR EL ENUNCIADO. NO PUEDES DESCONECTAR A OTRO USUARIO, INDEPENDIENTEMENTE DE SI EXISTE O NO
run_test 11 disconnect_ntpe           $'CONNECT benito\nDISCONNECT doesnotexist'
run_test 12 publish_ok               $'CONNECT benito\nPUBLISH ./register_user.py "Mi fichero de prueba"'
run_test 13 publish_ok_2             $'CONNECT benito\nPUBLISH ./run_all.sh "Mi fichero de prueba 2"'
run_test 14 publish_dup              $'CONNECT benito\nPUBLISH ./register_user.py "otra desc"'
run_test 15 publish_nct              $'PUBLISH ./newfile.py "y ootra desc'
run_test 16 delete_ok                $'CONNECT benito\nDELETE ./register_user.py'
run_test 17 delete_not_pub           $'CONNECT benito\nDELETE ./register_user.py'
run_test 18 delete_not_cnt           "DELETE ./register_user.py"
run_test 19 list_users_ok            $'REGISTER user_getfile\nCONNECT user_getfile\nLIST_USERS'
run_test 20 list_users_nct           "LIST_USERS"
run_test 21 list_content_ok          $'CONNECT user_getfile\nLIST_CONTENT benito'
run_test 22 list_content_nct         "LIST_CONTENT benito"
run_test 23 list_content_rdne        $'CONNECT user_getfile\nLIST_CONTENT doesnotexist'
run_getfile_parallel 24 get_file_ok \
    ./run_all.sh            \
    ./tmp_copy.md           \
    $'CONNECT benito'       \
    $'REGISTER user_getfile\nCONNECT user_getfile\nGET_FILE benito ./run_all.sh ./tmp_copy.md'
run_test 25 get_file_nce "GET_FILE benito ./run_all.sh ./tmp_copy.md"
run_getfile_fail 26 get_file_fne \
    ./tmp_copy.md \
    $'REGISTER paco\nCONNECT paco\nPUBLISH ../../.gitignore "esto es una descripcion"' \
    $'CONNECT user_getfile\nGET_FILE paco ./doesnotexist.sh ./tmp_copy.md'

rm -rf temp