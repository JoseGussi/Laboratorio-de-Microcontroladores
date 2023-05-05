import serial, csv

lectura_serial = serial.Serial("COM2", 9600) #Comunicacion con la placa mediante el puerto ttyS0

filename= open("Voltajes.csv",'w') 
escritura_archivo = csv.writer(filename)

while (True):
	data = lectura_serial.readline().decode().split(' ')
	escritura_archivo.writerow(data)

filename.close()
