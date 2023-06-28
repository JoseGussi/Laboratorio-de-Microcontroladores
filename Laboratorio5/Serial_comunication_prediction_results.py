#!/usr/bin/python3

import sys
import serial
import time
import csv


# Configuración de la conexión serial
data = []
serial_port = "/dev/ttyACM0"
baud_rate = 9600
header = ['Predicciones porcentuales']
samples = 2000
counter = 0
filename =  'predictions.csv'

ser = serial.Serial(serial_port, baud_rate, timeout=1)
print("Connected to MCU")


file = open(filename, 'w', encoding='UTF8')
writeFile  = csv.writer(file)
writeFile.writerow(header)

while(counter < samples):
    
	# Lectura del puerto serial
    data = ser.readline().decode("utf-8").replace('\r', "")
    #print(data)
    data = data.split('\n')
#    print(len(data))

    if(True):    
        writeFile.writerow(data)
        print(data)
        
        counter+=1
