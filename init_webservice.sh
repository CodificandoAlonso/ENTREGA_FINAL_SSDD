python3 ./src/client/ws-time.py -p 4567 &
sleep 3
python3 -mzeep http://localhost:4567/?wsdl

echo "Servicio web iniciado"

