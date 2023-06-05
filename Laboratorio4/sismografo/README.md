# README
Para la ejecución, revisión y pruebas de este Laboratorio 04 se debe tomar en cuenta que la carpeta principal del código del repositorio /src  para la configuración del MCU se debe encontrar en la dirección del folder raíz de la librería libopencm3 en la carpeta del microcontrolador /libopencm3-examples/examples y ejecutar el make bin del archivo Makefile en la raíz ./libo/libopencm3-examples para generar el archivo en binario. 
## Make sure
that, you can type "make clean; make" in any of the individual project
directories later:

    cd examples/stm32/f1/stm32-h103/miniblink
    make flash V=1


## Archivo principal 
DEbido a que el código es basado en los ejemplos de la librería se utilizó de archivo base el archivo lcd-serial.c 

