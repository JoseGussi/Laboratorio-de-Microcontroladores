
#include <PCD8544.h>

const int analogPin = A0;
const float Vref = 5.0;
const int ADCResolution = 1023;
const float R1 = 1000.0;
const float R2 = 10000.0;

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
  
}

void loop() {
  int adcValue = analogRead(analogPin); //Reads analog input
  float voltage = ((511.0 - adcValue)) * 49.0/1023.0; //Substracts the 2.5V offset, to start in 0, then retrieves the real value

  lcd.setCursor(0, 0);

  lcd.print("V 1: ");
  lcd.print(voltage);
  //lcd.print("V");
}
