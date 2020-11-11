//Gustavo Angel Toledo Nin
//Universidad Áutonoma de Ciudad Juárez

//Librerias para el proyecto
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> // Repositorio de libreria: https://github.com/Links2004/arduinoWebSockets/releases o dentro de la carpeta de github se agrego también.
#include <ArduinoJson.h> // Repositorio de libreria: https://arduinojson.org/ o bien instalar por medio del administrador de librerias, instalar la 5.13.4 OBLIGARIO.
#include <StreamString.h>
#include <Hash.h>
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

WiFiClient client;
#define MyApiKey "xxxxxxxxxxxxxxxxxxxx" // Aqui se pone la llave que te da sinric.com con respecto a cada usuario.
#define MySSID "xxxxxxxxxxxxxxxxxxxxx" // El nombre de tu internet.
#define MyWifiPassword "xxxxxxxxxxxxxxxxxxxxxx" // Contraseña de tu internet.
int device1 = 5;
#define HEARTBEAT_INTERVAL 300000 // 5 minutos

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void turnOn(String deviceId) {
 if (deviceId == "xxxxxxxxxxxxxxxxxxxxxx") // ID del primer dispositivo: El sistema.
 { 
 Serial.print("Se ha encendido el sistema...\n");
 digitalWrite(device1, HIGH);
 }else if (deviceId == "xxxxxxxxxxxxxxxxxxxxxxxxx") // ID del segundo dispositivo.
 { 
  Serial.print("Turn on device id: ");
  Serial.println(deviceId);
 }
 else{ 
 Serial.print("Turn on for unknown device id: ");
 Serial.println(deviceId); 
 } 
}

void turnOff(String deviceId) {
 if (deviceId == "xxxxxxxxxxxxxxxxxxxxxx") // Device ID of device 1
 { 
 Serial.print("Se ha apagado el sistema...\n");
 digitalWrite(device1, LOW);
 }
 else if (deviceId == "xxxxxxxxxxxxxxxxxxxxxx") // ID del segundo dispositivo sección apagado.
 { 
  Serial.print("Turn on device id: ");
  Serial.println(deviceId);
 }
 else {
 Serial.print("Turn off for unknown device id: ");
 Serial.println(deviceId); 
 }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
 switch(type) {
 case WStype_DISCONNECTED:
 isConnected = false; 
 Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
 break;
 case WStype_CONNECTED: {
 isConnected = true;
 Serial.printf("[WSc] Servicio conectado a sinric.com por url: %s\n", payload);
 Serial.printf("Paso 4. Por favor dar instrucciones ...\n"); 
 }
 break;
 case WStype_TEXT: {
 //Serial.printf("[WSc] get text: %s\n", payload);

 DynamicJsonBuffer jsonBuffer;
 JsonObject& json = jsonBuffer.parseObject((char*)payload); 
 String deviceId = json ["deviceId"]; 
 String action = json ["action"];
 
 if(action == "setPowerState") { // Switch or Light
 String value = json ["value"];
 if(value == "ON") {
 turnOn(deviceId);
 } else {
 turnOff(deviceId);
  }
 }
 else if (action == "SetTargetTemperature") {
 String deviceId = json ["deviceId"]; 
 String action = json ["action"];
 String value = json ["value"];
 }
 else if (action == "test") {
 Serial.println("[WSc] received test command from sinric.com");
 }
 }
 break;
 case WStype_BIN:
 Serial.printf("[WSc] get binary length: %u\n", length);
 break;
 }
}

void setup() 
{
 pinMode (device1, OUTPUT);
 digitalWrite(device1,LOW);
 Serial.begin(9600);
 
 WiFiMulti.addAP(MySSID, MyWifiPassword);
 Serial.println();
 Serial.print("Paso 1. Conectando al SSID: ");
 Serial.println(MySSID);

// Waiting for Wifi connect
 while(WiFiMulti.run() != WL_CONNECTED) {
 delay(500);
 Serial.print("Conectando, por favor espere ...\n");
 }
 if(WiFiMulti.run() == WL_CONNECTED) {
 Serial.println("");
 Serial.print("Paso 2. WiFi conectado. ");
 Serial.print("\nPaso 3. Dirección ip asignada: ");
 Serial.println(WiFi.localIP());
 }

// server address, port and URL
 webSocket.begin("iot.sinric.com", 80, "/");

// event handler
 webSocket.onEvent(webSocketEvent);
 webSocket.setAuthorization("apikey", MyApiKey);
 
 // try again every 5000ms if connection has failed
 webSocket.setReconnectInterval(5000); 
}

void loop() {
 
 webSocket.loop();
 
 if(isConnected) {
 uint64_t now = millis();
 
 // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
 if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
 heartbeatTimestamp = now;
 webSocket.sendTXT("H"); 
  }
 } 
}
