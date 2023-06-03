#!/usr/bin/python3


import serial
import paho.mqtt.client as mqtt
import json, time
import ssl, socket





# Configuración de la conexión serial
serial_port = "/dev/ttyACM0"
baud_rate = 115200
ser = serial.Serial(serial_port, baud_rate, timeout=1)
print("Connected to MCU")






# Callback que se ejecuta al conectarse al broker MQTT
def on_connect(client, userdata, flags, rc):
    print("Conectado al broker:", client._host)
    client.subscribe(topic)

def on_disconnect(client, userdata, rc):
    if(rc == 0):
        print("Client disconneted OK")
    else:
        print("System disconnected via code: ", rc)


# Callback que se ejecuta al recibir un mensaje MQTT
def on_message(client, userdata, msg):
    print("Mensaje recibido:", msg.topic, msg.payload.decode())


# Configuración del cliente MQTT
broker = "iot.eie.ucr.ac.cr"
port = 1883
topic ='v1/devices/me/telemetry'
client_id = "d7215990-001e-11ee-9c20-cd0367d667ec"
username = "Device_stm32f429i_JC_JM"
password = "470xtxejzpo2mmdqjram"
button = {"enabled: ": False}



# Inicialización del cliente MQTT
client = mqtt.Client()
client.connected = False
client.username_pw_set(password)
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect





# Conexión al broker MQTT
client.connect(broker, port)
Datapkg = dict()
# Envío de comandos y lectura del puerto serial
try:
    while True:
        # Lectura del puerto serial
        
        
            data = ser.readline().strip().decode("utf-8").replace('\r', "").replace('\n', "")
            data = data.split('\t')

            if(len(data) > 3):
                Datapkg["x"] = data[0]
                Datapkg["y"] = data[1]
                Datapkg["z"] = data[2]
                Datapkg["Battery"] = data[3]

                if(data[3] == "1"):
                    alarm = 0
                else:
                    alarm = 1
                Datapkg["BatteryAlarm"] = alarm
            #print(data)

            #print("Dato recibido por el puerto serial X:", data)
            #print("Dato recibido por el puerto serial Y:", data[1])
            #print("Dato recibido por el puerto serial Z:", data[2])

            # Envío del dato al broker MQTT
            
            payload = {"data": data}
            output = json.dumps(Datapkg)
            #print(output)
            client.publish(topic, output)
            client.loop()

except KeyboardInterrupt:
    pass

# Finalización del programa

client.disconnect()
ser.close()













