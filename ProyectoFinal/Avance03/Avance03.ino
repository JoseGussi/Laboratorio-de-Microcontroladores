#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include "esp_camera.h"

// Inicializa Bot Telegram
#define BOTtoken "6203517484:AAE9qgnCQEj7QHX56B6H3dnD7fysqqfhGZo"  // Tu Bot Token (Obtener de Botfather)
#define CAMERA_MODEL_AI_THINKER
#define CHAT_ID "1076728481"

#define FLASH_LED_PIN 4

#include "camera_pins.h"

Servo myservo;  // crea el objeto servo



// GPIO de salida del servo
static const int servoPin = 12;

float DISTANCIA = 0.0;
String mensaje = "";
String IP = "";
static const int pinEco=15;
static const int triggerPin=13;

// Reemplazar con tus credenciales de Wifi
const char* ssid     = "502 bad gateway";
const char* password = "Eskejoje2801";







void startCameraServer();


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

String porcentaje(float distancia)
{
  float porcentaje;
  Serial.print("Distancia");
  Serial.println(distancia);
  porcentaje = 100 - ((distancia/18) *100);
  Serial.print("Porcentaje");
  Serial.println(porcentaje);
  String tmp = String(porcentaje);
  Serial.println((distancia/18)*100);
  return tmp;

}

void handleNewMessages(int numNewMessages, float distancia, String ipString) {

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    String text = bot.messages[i].text;



    if (text == "/comida") {
      
      //myservo.attach(servoPin);  // vincula el servo en el servoPin
      bot.sendMessage(chat_id, "Alimentando", "");
      
      myservo.write(190); 
      Serial.println("Me mov[i");
      delay(3000); 
      myservo.write(90); 
      Serial.println("Me movi");
      text = "listo" ;
    
    }

    if (text == "/cantidad") {
      Serial.println(porcentaje(distancia));
       // Crea un mensaje con el valor de la variable
      String mensaje = "El contenedor tiene un  " + porcentaje(distancia)+"% de alimento" ;
      // Envía el mensaje al chat
      bot.sendMessage(chat_id, mensaje);
      text = "listo" ;
    }
    if (text == "/camara"){
      //setup_camera();
      mensaje = "Camara Lista! Puedes acceder a la camara con el siguiente link para hacer stream 'http:// " + ipString +  "' Puedes pegarlo en un navegador y hacer stream";
      Serial.println(mensaje);
      Serial.println(WiFi.localIP());
      text = "listo" ;
      bot.sendMessage(chat_id, mensaje);
      
    }
  }
}


void startCameraServer(); 

String setup_camera() {
  Serial.setDebugOutput(true);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
    // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return IP;
    
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);  
  
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
   
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  // Obtener la dirección IP local
  IPAddress ip = WiFi.localIP();

  // Convertir la dirección IP en texto
  IP = ip.toString();
  Serial.println(IP);
  return IP;
}


void setup() {
  Serial.begin(115200);
  

  
  // Conecta a red WiFi con SSID y password
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  

  // Muestra IP local 
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  myservo.attach(servoPin);  // vincula el servo en el servoPin
  bot.sendMessage(CHAT_ID, "Bievenido al Bot para Cuidar a tu mascota \n tienes varias opciones escribiendo los siguientes mensajes para darle cuidados a tu mascota: \n 1. Alimentarla: /comida \n 2. Comida disponible: /cantidad  \n 3. Observar como está: /camara", "");
  setup_camera();

  cdmyservo.attach(servoPin);  // vincula el servo en el servoPin
}

void loop() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  //Calculamos la distancia en cm
  DISTANCIA = 0.01723 * readUltrasonicDistance(triggerPin, pinEco);
  //Mostramos la disstancia
  Serial.println(DISTANCIA);
  delay(1000);
 

  while(numNewMessages) {
    handleNewMessages(numNewMessages, DISTANCIA, IP);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
