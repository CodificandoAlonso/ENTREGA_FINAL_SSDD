#!/usr/bin/env python3
import random
import socket, uuid, sys, argparse
from zeep import Client

class RegisterUserTestCase:
    def __init__(self, server, port, ws_port):
        self.server  = server
        self.port    = port
        self.ws_port = ws_port
        wsdl_url     = f"http://127.0.0.1:{ws_port}/?wsdl"
        try:
            self.soap = Client(wsdl=wsdl_url)
        except Exception as e:
            print(f"[ERROR] No puedo conectar al WSDL {wsdl_url}: {e}", file=sys.stderr)
            sys.exit(1)
    def send_op_and_user(self, sock, op, user):
        sock.connect((self.server, self.port))
        sock.sendall(op.encode() + b'\0')

        timestamp = self.soap.service.get_time()
        sock.sendall(timestamp.encode() + b'\0')
        sock.sendall(user.encode() + b'\0')
        if op == "CONNECT":
            port = random.randint(1025, 65535)
            sock.sendall(str(port).encode() + b'\0')

    def register_and_connect_user(self):
        username = str(uuid.uuid4())
        with socket.socket() as s:
            try:
                self.send_op_and_user(s, "REGISTER", username)
                rcv = s.recv(1)
            except Exception as e:
                print(f"[ERROR] Registro fallo: {e}")
                return

            if not rcv:
                print("ERROR: no recibí respuesta")
            elif rcv[0] == 0:
                print("REGISTER OK", flush=True)
            else:
                print("REGISTER ERROR", flush=True)
                return
            s.close()
        with socket.socket() as s:
            try:
                self.send_op_and_user(s, "CONNECT", username)
                rcv = s.recv(1)
            except Exception as e:
                print(f"[ERROR] Connect fallo: {e}")
                return
            if not rcv:
                print("ERROR: no recibí respuesta")
            elif rcv[0] == 0:
                print("CONNECT OK", flush=True)
            else:
                print("CONNECT ERROR", flush=True)
                return


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s","--server",   required=True)
    parser.add_argument("-p","--port",     type=int, required=True)
    parser.add_argument("-ws","--ws-port", type=int, required=True)
    args = parser.parse_args()

    tester = RegisterUserTestCase(args.server, args.port, args.ws_port)
    # haz 10 registros
    for _ in range(10):
        tester.register_and_connect_user()

if __name__ == "__main__":
    main()