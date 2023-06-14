# README
Para la ejecución, revisión y pruebas de este Laboratorio 04 se debe tomar en cuenta que la carpeta principal del código del repositorio /src  para la configuración del MCU se debe encontrar en la dirección del folder raíz de la librería libopencm3 en la carpeta del microcontrolador /libopencm3-examples/examplesstm32/f4/stm32f429i-discovery/examples y ejecutar el make bin del archivo Makefile en la raíz ./libo/libopencm3-examples para generar el archivo en binario. 
## Make sure
Se debe aegurar que se corran los 

    cd /libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/examples
    COPIAR CARPERTA
Copiar a carpeta con el código fuente src/:

    
    cp -r src  ./libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/examples
    cd src/ 
## Correr el codigo y cargar al MCU
Con el folder src  dentro del directorio de examples del microcontrolador ejecutar en la raíz  /libopencm3-examples, compilar los archivos y generar los binarios 

    cd ./libopencm3-examples/
    make bin
### Archivo principal
Debido a que el código es basado en los ejemplos de la librería se utilizó de archivo base el archivo lcd-serial.c 
Por lo tanto, para enviar el archivo principal para correrlo en el MCU es el archivo generado por el lcd-serial.c, es decir lcd-serial.bin
El comando para cargar el programa al MCU es: 

        
    st-flash lcd-serial.bin 0x08000000
    
### Script de Python 

El script de python se puede correr desde cualquier terminal en la ubicación dándole permisos de ejecución: 

    chmod +x comunication.py
    ./comunication.py
    

