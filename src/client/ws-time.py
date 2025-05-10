import time
import logging
from wsgiref.simple_server import make_server
from spyne import Application, ServiceBase, rpc, String
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication


class GetTime(ServiceBase):
    @rpc(_returns=String)
    def get_time(self):
        this_time = time.time()
        local_time = time.localtime(this_time)
        formatted_time = time.strftime("%H:%M:%S %d/%m/%Y", local_time)
        print(formatted_time)
        return formatted_time

application = Application(
    services=[GetTime],
    name="TimeService",
    tns='http://example.com/time-service',
    in_protocol=Soap11(validator='lxml'),
    out_protocol=Soap11()
)

application = WsgiApplication(application)

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    server = make_server('127.0.0.1', 8000, application)
    print("Servidor SOAP ejecutándose en http://127.0.0.1:8000")
    server.serve_forever()