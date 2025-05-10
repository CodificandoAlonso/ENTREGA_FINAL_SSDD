
#include <rpc/rpc.h>
#include "server-rpc-builder.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern void rpc_service_datetime_1(struct svc_req *rqstp, SVCXPRT *transp);

bool_t
print_datetime_1_svc(entry *argp, int  *resultp, struct svc_req *rqstp)
{
    (void)rqstp;
    if (strcmp(argp->operation, "PUBLISH")){
          printf("%s\t %s %s\t %s",argp->username, argp->operation, argp->filename, argp->datetime);
    }
    *resultp = 0;
    return TRUE;
}

bool_t
rpc_service_datetime_1_freeresult(SVCXPRT *transp,
                                 xdrproc_t xdr_res,
                                 char *resultsp)
{
    xdr_free(xdr_res, resultsp);
    return TRUE;
}



int main() {
    register SVCXPRT *transp;

    pmap_unset(RPC_SERVICE_DATETIME, RPC_SERVICE);

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (!transp) {
        fprintf(stderr, "No puedo crear servicio TCP\n");
        exit(1);
    }

    if (!svc_register(transp, RPC_SERVICE_DATETIME, RPC_SERVICE,
                      rpc_service_datetime_1, IPPROTO_TCP)) {
        fprintf(stderr, "No puedo registrar RPC_SERVICE_DATETIME\n");
        exit(1);
                      }

    svc_run();  /* bucle de servicio RPC */
    fprintf(stderr, "svc_run retornó\n");
    return 1;
}