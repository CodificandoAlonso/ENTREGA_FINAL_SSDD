# Compilación y Ejecución

## Entorno de Desarrollo

**Plataforma principal:**
`guernika-gpu.lab.inf.uc3m.es`

**Dependencias:**

* Compilador C
* Python 3
* Bibliotecas estándar
* ONC-RPC

---

## Proceso de Compilación

| Acción            | Comando      | Descripción                                 |
| ----------------- | ------------ | ------------------------------------------- |
| Compilar proyecto | `make`       | Genera ejecutables del servidor P2P y RPC   |
| Limpiar sistema   | `make clean` | Elimina base de datos y archivos temporales |

---

## Secuencia de Ejecución

### 1. Servicio Web

**Propósito:** Proveer hora/fecha vía SOAP

**Pasos:**

1. Ejecutar servicio:

   ```bash
   python3 ./src/client/ws-time.py -p <PUERTO>
   ```
2. Validar WSDL:

   ```bash
   python3 -m zeep http://localhost:<PUERTO>/?wsdl
   ```

**Atajo:**

```bash
./init_webservice.sh  # Usa puerto 4567 por defecto
```

---

### 2. Servidor RPC

**Requisito previo:**

```bash
export LOG_RPC_IP=localhost  # Exclusivo para Guernika
```

**Inicio:**

```bash
./servidor_rpc
```

---

### 3. Servidor P2P

**Sintaxis:**

```bash
./servidor -p <PUERTO>
```

**Ejemplo:**

```bash
./servidor -p 3333
```

---

### 4. Cliente P2P

**Parámetros principales:**

| Flag  | Descripción             | Valor por defecto |
| ----- | ----------------------- | ----------------- |
| `-s`  | IP del servidor         | `localhost`       |
| `-p`  | Puerto del servidor     | `3333`            |
| `-ws` | Puerto del servicio web | `4567`            |

**Ejemplo completo:**

```bash
python3 ./src/client/client.py -s localhost -p 3333 -ws 4567
```

---

## Herramientas de Soporte

| Script               | Ubicación         | Función                                   |
| -------------------- | ----------------- | ----------------------------------------- |
| `stress_test.sh`     | Raíz del proyecto | Simula carga concurrente de usuarios      |
| `delete_database.sh` | Raíz del proyecto | Elimina la base de datos SQLite           |
| `run_all.sh`         | `src/tests/`      | Ejecuta las 26 pruebas funcionales        |
| `register_user.py`   | `src/tests/`      | Crea usuarios aleatorios para las pruebas |

---

# Batería de Pruebas

## Prueba de Estrés

**Objetivo:** Verificar estabilidad bajo carga.

**Condiciones:**

* 10 conexiones concurrentes
* Usuarios con UUID únicos
* Puertos aleatorios entre 1025 y 65535
* Registros simultáneos

**Resultado esperado:**

* 0 colisiones IP\:PORT en la base de datos

---

## Pruebas Funcionales

**Cobertura:** 26 escenarios (tanto exitosos como con errores)

### Flujo típico:

1. Limpiar la base de datos:

   ```bash
   ./delete_database.sh
   ```
2. Ejecutar pruebas:

   ```bash
   cd src/tests/
   ./run_all.sh
   ```
3. Comparar salidas:

   * Resultados en `temp/`
   * Esperados en `expected/`

### Casos destacados:

| Nº Test | Nombre            | Observaciones                         |
| ------- | ----------------- | ------------------------------------- |
| 19      | `list_user_ok`    | Validar formato de IP en la salida    |
| 21      | `list_content_ok` | Ruta absoluta dependiente del entorno |

---

## Notas Técnicas

* Todas las pruebas requieren reinicio de base de datos.
* Tests 19 y 21 pueden necesitar ajustes si se cambia de entorno.
* El puerto 4567 es clave para la integración web/RPC.

---
