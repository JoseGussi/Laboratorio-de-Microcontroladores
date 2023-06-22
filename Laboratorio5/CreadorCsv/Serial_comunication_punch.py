#!/usr/bin/python3

import sys
import serial
import time
import csv


# Configuración de la conexión serial
data = []
serial_port = "/dev/ttyACM0"
baud_rate = 9600
header = ['aX', 'aY', 'aZ', 'gX', 'gY', 'gZ']
samples = 2000
counter = 0
filename =  'punch.csv'

ser = serial.Serial(serial_port, baud_rate, timeout=1)
print("Connected to MCU")


file = open(filename, 'w', encoding='UTF8')
writeFile  = csv.writer(file)
#writeFile.writerow(header)

while(counter < samples):
    
	# Lectura del puerto serial
    data = ser.readline().decode("utf-8").replace('\r', "").replace('\n', "")
   # print(data)
    data = data.split(',')
   # print(len(data))

    if(len(data) == 6):    
        writeFile.writerow(data)
       # print(data)
        
        counter+=1
