#include <errno.h>
#include <pthread.h>
#include <rpc/rpc.h>
#include "serverRpcBuilder.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MAX_THREADS 25

static pthread_t workers[MAX_THREADS];
static struct task *queue[MAX_THREADS * 8];
static int head = 0, tail = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t mutex_workload = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mutex_copy_cond = PTHREAD_COND_INITIALIZER;
int mutex_copy_params = 0;

struct task {
    SVCXPRT *xprt;
    entry args;
};


/**
 * @brief Funcion que realizan todos los hilos del servicio RPC. En este caso se quedan esperando cuando no hay
 * elementos encolados. En cuanto haya lgun elemento, todos los hilos "pelearan" por entrar dentro de la region
 * critica y se encargarán de imprimir la estructura task(sus parametros) por pantalla. Luego liberaran toda esa
 * region para que no se produzca ni goteo de memoria, ni bloqueo de proceso por falta de memoria en el heap.
 */
static void *handler_thread(void *unused) {
    (void) unused; //Se necesita un atributo, para que no de warning se convierte a nada
    pthread_mutex_lock(&mutex_workload);
    mutex_copy_params = 1;
    pthread_cond_signal(&mutex_copy_cond);
    pthread_mutex_unlock(&mutex_workload);

    for (;;) {
        pthread_mutex_lock(&mutex);
        //Si head = tail quiere decir que no hay nada encolado, esperas a que la funcion print_datetima_svc_1
        //indique que ha copiado y guardado algo en la cola
        while (head == tail)
            pthread_cond_wait(&cond, &mutex);
        struct task *t = queue[head];
        head = (head + 1) % (MAX_THREADS * 8);
        pthread_mutex_unlock(&mutex);


        if (strcmp(t->args.operation, "PUBLISH") == 0) //Si publish se imprime tmb el filename
            printf("%s    %s %s    %s\n", t->args.username,
                   t->args.operation, t->args.filename, t->args.datetime);
        else
            printf("%s    %s    %s\n", t->args.username,
                   t->args.operation, t->args.datetime);

        //Liberacion de todas las cosas que se han hecho con strdup
        free(t->args.username);
        free(t->args.operation);
        free(t->args.datetime);
        free(t->args.filename);
        free(t);
    }
    return NULL;
}


//servicio externo del RPC service
extern void rpc_service_datetime_1(struct svc_req *rqstp, SVCXPRT *transp);

/**
 * @brief Funcion encargada de realizar el servicio requerido. En este caso hace una copia local de la estructura
 * entry del mensaje, la copia en una estructura task y la inserta en la cola, avisando de que ya hay algun elemento
 * en esta, de tal forma que alguno de los hilos se encargara de recogerla e imprimir por pantalla.
 */
bool_t
print_datetime_1_svc(entry *argp, int *resultp, struct svc_req *rqstp) {
    struct task *t = malloc(sizeof *t);
    if (!t) {
        perror("malloc");
        *resultp = -1;
        return FALSE;
    }
    //Copia local. Luego hay que hacer free de cada cosa concreta hecha con strdup
    t->args.username = strdup(argp->username);
    t->args.operation = strdup(argp->operation);
    t->args.filename = strdup(argp->filename);
    t->args.datetime = strdup(argp->datetime);
    t->xprt = rqstp->rq_xprt;

    pthread_mutex_lock(&mutex);
    queue[tail] = t;
    tail = (tail + 1) % (MAX_THREADS * 8);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    //Una vez se haya insertado en la cola, se puede llamar a la liberacion de la estructura entry y devolver resultado
    //de exito
    svc_freeargs(rqstp->rq_xprt, (xdrproc_t)xdr_entry, (char *)argp);
    *resultp = 0;
    return TRUE;
}

/**
 * @brief Funcion encargada de liberar los posibles buffers de memoria reservada para las funciones
 */
bool_t
rpc_service_datetime_1_freeresult(SVCXPRT *transp,
                                  xdrproc_t xdr_res,
                                  char *resultsp) {
    (void) transp;
    xdr_free(xdr_res, resultsp);
    return TRUE;
}


int main() {
    //Iniciar el servicio del servidor
    register SVCXPRT *transp;
    pmap_unset(RPC_SERVICE_DATETIME, RPC_SERVICE);


    //Protocolo TCP
    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (!transp) {
        fprintf(stderr, "No puedo crear servicio TCP\n");
        exit(1);
    }
    //Registro de servicio TCP
    if (!svc_register(transp, RPC_SERVICE_DATETIME, RPC_SERVICE,
                      rpc_service_datetime_1, IPPROTO_TCP)) {
        fprintf(stderr, "No puedo registrar RPC_SERVICE_DATETIME\n");
        exit(1);
    }

    //Protocolo UDP
    transp = svcudp_create(RPC_ANYSOCK);
    if (!transp) {
        fprintf(stderr, "No puedo crear servicio UDP\n");
        exit(1);
    }

    //Registro de servicio UDP
    if (!svc_register(transp, RPC_SERVICE_DATETIME, RPC_SERVICE,
                      rpc_service_datetime_1, IPPROTO_UDP)) {
        fprintf(stderr, "No puedo registrar RPC_SERVICE_DATETIME\n");
        exit(1);
    }

    //Creacion de hilos que esperarán a que haya operaciones en la cola
    for (int i = 0; i < MAX_THREADS; ++i) {
        pthread_create(&workers[i], NULL, (void *) handler_thread, NULL);
        pthread_mutex_lock(&mutex_workload);
        while (mutex_copy_params == 0)
            pthread_cond_wait(&mutex_copy_cond, &mutex_workload);
        mutex_copy_params = 0;
        pthread_mutex_unlock(&mutex_workload);
        pthread_detach(workers[i]);
    }

    //Bucle de recepción de solicitudes
    fd_set rfds;
    for (;;) {
        rfds = svc_fdset;
        int nfds = select(svc_maxfd + 1, &rfds, NULL, NULL, NULL);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        //Esta funcion implementada por los stubs de RPC se encarga de llamar a print_datetime_svc_1
        svc_getreqset(&rfds);
    }
    return 0;
}
