
#include <PCD8544.h>

const int analogPin = A0;
const int analogPin1 = A1;
const int analogPin2 = A2;
const int analogPin3 = A3;
const int PinSwitch = A5;
const int pinBoton = 5;
bool estadoBoton;
float voltage = 0;
float voltage1 = 0;
float voltage2 = 0;
float voltage3 = 0;
float vRMS = 0;
float vRMS1 = 0;
float vRMS2 = 0;
float vRMS3 = 0;
float incomingByte = 0; // entrada para el serial port

const int numMuestras = 60; // Número de muestras que deseas tomar
const int pinSenal = A0; // Pin analógico al que está conectada la señal
const int intervaloMuestreo = 1; // Intervalo de tiempo entre muestras en milisegundos
float muestras[numMuestras]; // Vector para almacenar las muestras
float muestras1[numMuestras]; 
float muestras2[numMuestras]; 
float muestras3[numMuestras]; 


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
  pinMode(8, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
  pinMode(pinBoton, INPUT_PULLUP);
}


//Tomar muestras canal 1
void tomarMuestras() {
  for (int i = 0; i < numMuestras; i++) {
    muestras[i] = analogRead(analogPin);
    delay(intervaloMuestreo);
  }
}

//Tomar muestras canal 2
void tomarMuestras1() {
  for (int i = 0; i < numMuestras; i++) {
    muestras1[i] = analogRead(analogPin1);
    delay(intervaloMuestreo);
  }
}

//Tomar muestras canal 3
void tomarMuestras2() {
  for (int i = 0; i < numMuestras; i++) {
    muestras2[i] = analogRead(analogPin2);
    delay(intervaloMuestreo);
  }
}

//Tomar muestras canal 4
void tomarMuestras3() {
  for (int i = 0; i < numMuestras; i++) {
    muestras3[i] = analogRead(analogPin3);
    delay(intervaloMuestreo);
  }
}

//Calcular Vrms canal 1
float calcularVRMS() {
  float sumaCuadrados = 0;
  for (int i = 0; i < numMuestras; i++) {
    //float voltaje = (muestras[i] / 1023.0) * 5.0; // Convierte la muestra a voltaje (asumiendo una referencia de 5V)
    float voltajeA = ((511.0 - muestras[i])) * 55.0/1023.0;
    sumaCuadrados += pow(voltajeA, 2); // Eleva al cuadrado y suma al acumulador
  }
  float promedioCuadrados = sumaCuadrados / numMuestras; // Calcula el promedio de los cuadrados
  vRMS = sqrt(promedioCuadrados); // Calcula la raíz cuadrada del promedio
  return vRMS;
}


//Calcular Vrms canal 2
float calcularVRMS1() {
  float sumaCuadrados1 = 0;
  for (int i = 0; i < numMuestras; i++) {
    //float voltaje = (muestras[i] / 1023.0) * 5.0; // Convierte la muestra a voltaje (asumiendo una referencia de 5V)
    float voltajeA1 = ((511.0 - muestras1[i])) * 55.0/1023.0;
    sumaCuadrados1 += pow(voltajeA1, 2); // Eleva al cuadrado y suma al acumulador
  }
  float promedioCuadrados1 = sumaCuadrados1 / numMuestras; // Calcula el promedio de los cuadrados
  float vRMS1 = sqrt(promedioCuadrados1); // Calcula la raíz cuadrada del promedio
  return vRMS1;
}


//Calcular Vrms canal 3
float calcularVRMS2() {
  float sumaCuadrados2 = 0;
  for (int i = 0; i < numMuestras; i++) {
    float voltajeA2 = ((511.0 - muestras2[i])) * 55.0/1023.0;
    sumaCuadrados2 += pow(voltajeA2, 2); // Eleva al cuadrado y suma al acumulador
  }
  float promedioCuadrados2 = sumaCuadrados2 / numMuestras; // Calcula el promedio de los cuadrados
  vRMS2 = sqrt(promedioCuadrados2); // Calcula la raíz cuadrada del promedio
  return vRMS2;
}

//Calcular Vrms canal 4
float calcularVRMS3() {
  float sumaCuadrados3 = 0;
  for (int i = 0; i < numMuestras; i++) {
    float voltajeA3 = ((511.0 - muestras3[i])) * 55.0/1023.0;
    sumaCuadrados3 += pow(voltajeA3, 2); // Eleva al cuadrado y suma al acumulador
  }
  float promedioCuadrados3 = sumaCuadrados3 / numMuestras; // Calcula el promedio de los cuadrados
  vRMS3 = sqrt(promedioCuadrados3); // Calcula la raíz cuadrada del promedio
  return vRMS3;
}


void loop() {
  //Lee el estado de boton de seleccion de modo
  bool estadoSwitch = digitalRead(PinSwitch);

  //Lee voltaje DC del primer canal
  if(estadoSwitch == 1){
    int adcValue1 = analogRead(analogPin); //Reads analog input
    voltage = ((511.0 - adcValue1)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value
    lcd.setCursor(0, 0);
    lcd.print("V1: ");
    lcd.print(voltage);
    lcd.print("V");
    // read the incoming byte:
    incomingByte = voltage;
   // say what you got:
    Serial.print("Voltage[DC]01: " );
    Serial.println( incomingByte);
    
  }
  else{
    //Lee voltaje AC del primer canal
    tomarMuestras(); // Toma las muestras de la señal sinusoidal
    vRMS = calcularVRMS(); // Calcula el valor RMS
    lcd.setCursor(0, 0);
    lcd.print("V1: ");
    lcd.print(vRMS);
    lcd.print("Vrms");
    incomingByte = vRMS;
    Serial.print("Voltage[AC]01: " );
    Serial.println( incomingByte);
  }
    //Enciende LED de precaucion del canal 1
    if (vRMS > 13.5 ||voltage > 20 || vRMS < -13.5 ||voltage < -20) { // si el valor de la variable supera los 20
    digitalWrite(8, HIGH); // enciende el LED
  } else {
    digitalWrite(8, LOW); // apaga el LED
  }
  
  
  
  //Lee voltaje DC del segundo canal
  if(estadoSwitch == 1){
    int adcValue2 = analogRead(analogPin1); //Reads analog input
    voltage1 = ((511.0 - adcValue2)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value
    lcd.setCursor(0, 1);
    lcd.print("V1: ");
    lcd.print(voltage1);
    lcd.print("V");
    incomingByte = voltage1;
    Serial.print("Voltage[DC]02: " );
    Serial.println( incomingByte);
    
     
  }
  else{
    //Lee voltaje AC del segundo canal
    tomarMuestras1(); // Toma las muestras de la señal sinusoidal
    vRMS1 = calcularVRMS1(); // Calcula el valor RMS
    lcd.setCursor(0, 1);
    lcd.print("V2: ");
    lcd.print(vRMS1);
    lcd.print("Vrms");
    incomingByte = vRMS1;
    Serial.print("Voltage[AC]02: " ) ;
    Serial.println( incomingByte) ;
  }
     //Enciende LED de precaucion del canal 2
    if (vRMS1 > 13.5 ||voltage1 > 20 || vRMS1 < -13.5 ||voltage1 < -20) { // si el valor de la variable supera los 20
    digitalWrite(12, HIGH); // enciende el LED
  } else {
    digitalWrite(12, LOW); // apaga el LED
  }


  
  //Lee voltaje DC del tercer canal
  if(estadoSwitch == 1){
    int adcValue3 = analogRead(analogPin2); //Reads analog input
    voltage2 = ((511.0 - adcValue3)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value
    lcd.setCursor(0, 2);
    lcd.print("V3: ");
    lcd.print(voltage2);
    lcd.print("V");
    incomingByte = voltage2;
    Serial.print("Voltage[DC]03 : " );
    Serial.println( incomingByte); 
     
  }
  else{
    //Lee voltaje AC del tercer canal
    tomarMuestras2(); // Toma las muestras de la señal sinusoidal
    float vRMS2 = calcularVRMS2(); // Calcula el valor RMS
    lcd.setCursor(0, 2);
    lcd.print("V3: ");
    lcd.print(vRMS2);
    lcd.print("Vrms");
    incomingByte = vRMS2;
    Serial.print("Voltage[AC]03: " );
    Serial.println( incomingByte);
  }
     //Enciende LED de precaucion del canal 3
    if (vRMS2 > 13.5 ||voltage2 > 20 || vRMS2 < -13.5 ||voltage2 < -20) { // si el valor de la variable supera los 20
    digitalWrite(2, HIGH); // enciende el LED
  } else {
    digitalWrite(2, LOW); // apaga el LED
  }





  //Lee voltaje DC del cuarto canal
  if(estadoSwitch == 1){
    int adcValue4 = analogRead(analogPin3); //Reads analog input
    voltage3 = ((511.0 - adcValue4)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value
    lcd.setCursor(0, 3);
    lcd.print("V4: ");
    lcd.print(voltage3);
    lcd.print("V");
    incomingByte = voltage3;
    Serial.print("Voltage[DC]04: " ) ;
    Serial.println( incomingByte) ;
    Serial.println(" ") ;
  }
  else{
    //Lee voltaje AC del cuarto canal
    tomarMuestras3(); // Toma las muestras de la señal sinusoidal
    vRMS3 = calcularVRMS3(); // Calcula el valor RMS
    lcd.setCursor(0, 3);
    lcd.print("V3: ");
    lcd.print(vRMS3);
    lcd.print("Vrms");
    incomingByte = vRMS3;
    Serial.print("Voltage[AC]04: " ) ;
    Serial.println( incomingByte) ;
    Serial.println(" ") ;
  }
     //Enciende LED de precaucion del canal 4
    if (vRMS3 > 13.5 ||voltage3 > 20 || vRMS3 < -13.5 ||voltage3 < -20) { // si el valor de la variable supera los 20
    digitalWrite(11, HIGH); // enciende el LED
  } else {
    digitalWrite(11, LOW); // apaga el LED
  }

  
 
    
 


  delay(500); // Espera un segundo antes de tomar más muestras
}
