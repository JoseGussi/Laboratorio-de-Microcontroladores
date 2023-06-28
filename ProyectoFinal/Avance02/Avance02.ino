#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

Servo myservo;  // crea el objeto servo

// GPIO de salida del servo
static const int servoPin = 14;

int DISTANCIA = 0;
static const int pinEco=12;
static const int triggerPin=13;

// Reemplazar con tus credenciales de Wifi
const char* ssid     = "QQice02";
const char* password = "monica321";

// Inicializa Bot Telegram
#define BOTtoken "6203517484:AAE9qgnCQEj7QHX56B6H3dnD7fysqqfhGZo"  // Tu Bot Token (Obtener de Botfather)


#define CHAT_ID "1076728481"

WiFiClientSecure client;

UniversalTelegramBot bot(BOTtoken, client);


long readUltrasonicDistance(int triggerPin, int echoPin)
{
  //Iniciamos el pin del emisor de reuido en salida
  pinMode(triggerPin, OUTPUT);
  //Apagamos el emisor de sonido
  digitalWrite(triggerPin, LOW);
  //Retrasamos la emision de sonido por 2 milesismas de segundo
  delayMicroseconds(2);
  // Comenzamos a emitir sonido
  digitalWrite(triggerPin, HIGH);
  //Retrasamos la emision de sonido por 2 milesismas de segundo
  delayMicroseconds(10);
  //Apagamos el emisor de sonido
  digitalWrite(triggerPin, LOW);
  //Comenzamos a escuchar el sonido
  pinMode(echoPin, INPUT);
  // Calculamos el tiempo que tardo en regresar el sonido
  return pulseIn(echoPin, HIGH);
}


String porcentaje(int distancia)
{
  int porcentaje;
  porcentaje = distancia/9 *100;
  String tmp = String(porcentaje);
  return tmp;

}


void handleNewMessages(int numNewMessages, int distancia) {

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    String text = bot.messages[i].text;

    if (text == "/comida") {
      bot.sendMessage(chat_id, "Alimentando", "");
      
      myservo.write(180); 
      delay(1000); 
      myservo.write(90);  
      delay(5000); 
      myservo.write(180); 
      delay(1000); 
      myservo.write(90);    
    }

    if (text == "/cantidad") {
      bot.sendMessage(chat_id, "Al contenedor"+porcentaje(distancia),  "");
      
    }
  }
}

void setup() {
  Serial.begin(115200);

  myservo.attach(servoPin);  // vincula el servo en el servoPin

  // Conecta a red WiFi con SSID y password
  Serial.print("Conectado a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);


  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Muestra IP local 
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot iniciado", "");
}

void loop() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  //Calculamos la distancia en cm
  DISTANCIA = 0.01723 * readUltrasonicDistance(triggerPin, pinEco);
  //Mostramos la disstancia
  Serial.println(DISTANCIA);
  delay(1000);

  while(numNewMessages) {
    handleNewMessages(numNewMessages, DISTANCIA);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
