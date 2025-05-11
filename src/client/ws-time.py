import time, logging, argparse
from wsgiref.simple_server import make_server, WSGIServer, WSGIRequestHandler
from socketserver import ThreadingMixIn
from spyne import Application, ServiceBase, rpc, String
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication


class GetTime(ServiceBase):
    @rpc(_returns=String)
    def get_time(self):
        this_time     = time.time()
        local_time    = time.localtime(this_time)
        formatted_time = time.strftime("%H:%M:%S %d/%m/%Y", local_time)
        print(f"[petición SOAP] Hora entregada: {formatted_time}")
        return formatted_time


#Se permiten multiples peticiones
class ThreadingWSGIServer(ThreadingMixIn, WSGIServer):
    daemon_threads = True


def main():
    parser = argparse.ArgumentParser(description="Servidor SOAP de hora actual")
    parser.add_argument("-p", "--port",
                        type=int, required=True,
                        help="Puerto donde levantar el servidor SOAP")
    args = parser.parse_args()

    application = Application(
        services=[GetTime],
        name="TimeService",
        tns='http://example.com/time-service',
        in_protocol=Soap11(validator='lxml'),
        out_protocol=Soap11()
    )
    wsgi_app = WsgiApplication(application)

    logging.basicConfig(level=logging.WARNING)
    server = make_server('127.0.0.1', args.port, wsgi_app,
                         server_class=ThreadingWSGIServer,
                         handler_class=WSGIRequestHandler)
    print(f"Servidor SOAP ejecutándose en http://127.0.0.1:{args.port}")
    server.serve_forever()


if __name__ == '__main__':
    main()
