# Compilación y Ejecución

## Entorno de Desarrollo
**Plataforma principal:**  
`guernika-gpu.lab.inf.uc3m.es`  
**Dependencias:**
- Compiladores C y Python 3
- Bibliotecas estándar + ONC-RPC

---

## Proceso de Compilación
| Acción               | Comando      | Efecto                                  |
|----------------------|--------------|-----------------------------------------|
| Compilar proyecto    | `make`       | Genera ejecutables del servidor P2P/RPC |
| Limpiar sistema      | `make clean` | Elimina base de datos y archivos temporales |

---

## Secuencia de Ejecución

### 1. Servicio Web
**Propósito:** Proveer hora/fecha vía SOAP  
**Pasos:**
1. Ejecutar servicio:  
   ```python3 ./src/client/ws-time.py -p <PUERTO>```
2. Validar WSDL:  
   ```python3 -mzeep http://localhost:<PUERTO>/?wsdl```

**Alternativa rápida:**  
```./init_webservice.sh``` (*Usa puerto 4567*)

---

### 2. Servidor RPC
**Requisito previo:**  
```export LOG_RPC_IP=localhost``` (*Exclusivo para Guernika*)  
**Inicio:**  
```./servidor_rpc```

---

### 3. Servidor P2P
**Sintaxis:**  
```./servidor -p <PUERTO>```  
**Ejemplo:**  
```./servidor -p 3333```

---

### 4. Cliente P2P
**Parámetros clave:**  
| Flag  | Descripción               | Valor por defecto |
|-------|---------------------------|-------------------|
| `-s`  | IP del servidor           | localhost         |
| `-p`  | Puerto del servidor       | 3333              |
| `-ws` | Puerto del servicio web   | 4567              |

**Ejemplo completo:**  
```python3 ./src/client/client.py -s localhost -p 3333 -ws 4567```

---

## Herramientas de Soporte

| Herramienta               | Ubicación          | Función                                  |
|---------------------------|--------------------|------------------------------------------|
| `stress_test.sh`          | Raíz del proyecto  | Simula carga concurrente de usuarios     |
| `delete_database.sh`      | Raíz del proyecto  | Elimina toda la base de datos SQLite     |
| `run_all.sh`              | `src/tests/`       | Ejecuta 26 pruebas funcionales          |
| `register_user.py`        | `src/tests/`       | Genera usuarios aleatorios para pruebas |

---

# Batería de Pruebas

## Prueba de Estrés
**Objetivo:** Verificar estabilidad bajo carga  
**Métrica:** 10 conexiones concurrentes con:
- Usuarios UUID
- Puertos aleatorios (1025-65535)
- Registros simultáneos

**Resultado esperado:**  
0 colisiones IP:PORT en base de datos

---

## Pruebas Funcionales
**Cobertura:** 26 escenarios (éxito/error)  
**Flujo típico:**
1. Limpiar BD con `delete_database.sh`
2. Ejecutar desde `src/tests/`:  
   ```./run_all.sh```
3. Comparar salidas en `temp/` vs `expected/`

**Casos destacados:**  
| Test | Nombre               | Consideración especial                   |
|------|----------------------|------------------------------------------|
| 19   | `list_user_ok`       | Validar formato de IP en salida          |
| 21   | `list_content_ok`    | Ruta absoluta dependiente del entorno    |

---

**Notas técnicas:**
- Todas las pruebas requieren reinicio de base de datos
- Los tests 19 y 21 pueden requerir ajustes según el entorno de ejecución
- El puerto 4567 es crítico para la integración web/RPC  