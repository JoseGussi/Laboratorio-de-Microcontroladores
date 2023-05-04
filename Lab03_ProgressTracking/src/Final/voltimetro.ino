
#include <PCD8544.h>

const int analogPin = A0;
const int PinSwitch = A5;
const float Vref = 5.0;
const float ADCResolution = 1023;
const int pinBoton = 5;
bool estadoBoton;




const int numMuestras = 100; // Número de muestras que deseas tomar
const int pinSenal = A0; // Pin analógico al que está conectada la señal
const int intervaloMuestreo = 1; // Intervalo de tiempo entre muestras en milisegundos
float muestras[numMuestras]; // Vector para almacenar las muestras



// Define los pines para conectar la pantalla LCD
#define SCLK 3
#define DIN 4
#define DC 5
#define CS 7
#define RST 6

static PCD8544 lcd;
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };

void setup() {
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
    // PCD8544-compatible displays may have a different resolution...
  lcd.begin(84, 48);

  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
  pinMode(pinBoton, INPUT_PULLUP);
}


void tomarMuestras() {
  for (int i = 0; i < numMuestras; i++) {
    muestras[i] = analogRead(analogPin);
    delay(intervaloMuestreo);
  }
}

float calcularVRMS() {
  float sumaCuadrados = 0;
  for (int i = 0; i < numMuestras; i++) {
    //float voltaje = (muestras[i] / 1023.0) * 5.0; // Convierte la muestra a voltaje (asumiendo una referencia de 5V)
    float voltajeA = ((511.0 - muestras[i])) * 56.0/1023.0;
    sumaCuadrados += pow(voltajeA, 2); // Eleva al cuadrado y suma al acumulador
  }
  float promedioCuadrados = sumaCuadrados / numMuestras; // Calcula el promedio de los cuadrados
  float vRMS = sqrt(promedioCuadrados); // Calcula la raíz cuadrada del promedio
  return vRMS;
}


void loop() {
  bool estadoSwitch = digitalRead(PinSwitch);

  estadoBoton = digitalRead(pinBoton);
  if(estadoSwitch == 1){
    int adcValue1 = analogRead(analogPin); //Reads analog input
    float voltage = ((511.0 - adcValue1)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value
    lcd.setCursor(0, 0);
    lcd.print("V1: ");
    lcd.print(voltage);
    lcd.print("V");
  }
  else{
    tomarMuestras(); // Toma las muestras de la señal sinusoidal
    float vRMS = calcularVRMS(); // Calcula el valor RMS
    lcd.setCursor(0, 0);
    lcd.print("V1: ");
    lcd.print(vRMS);
    lcd.print("V");
  }
  



  delay(1000); // Espera un segundo antes de tomar más muestras
}
