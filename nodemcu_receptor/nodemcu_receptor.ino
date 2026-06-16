#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

const char* ssid = "Santiago";          
const char* password = "45608056";  
const char* tbServer = "thingsboard.cloud";
const char* token = "i8zy38u28r2xpfz92kd8"; 

// Topic específico de la cátedra
const char* mqttTopicTelemetry = "utn/2026/c01/g01/telemetry"; 

SoftwareSerial arduinoSerial(5, 4); // D1 (Rx) y D2 (Tx)

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);
  
  // Búfer expandido para respuestas rápidas
  client.setBufferSize(512); 
  
  conectarWiFi();
  client.setServer(tbServer, 1883);
  client.setCallback(onMqttMessage); 
}

void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();

  // Escuchar la cadena JSON completa que manda el Arduino y replicarla en la nube
  if (arduinoSerial.available() > 0) {
    String payload = arduinoSerial.readStringUntil('\n');
    payload.trim();
    
    // Validamos que sea un JSON antes de publicar
    if (payload.startsWith("{") && payload.endsWith("}")) {
      client.publish("v1/devices/me/telemetry", payload.c_str());
      client.publish(mqttTopicTelemetry, payload.c_str());
    }
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) { message += (char)payload[i]; }
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (!error) {
    String method = doc["method"];
    
    // Despacho de comandos seriales según el control del Dashboard
    if (method == "setSetpoint") {
      int val = doc["params"];
      arduinoSerial.print("SET_R:"); arduinoSerial.println(val);
    } 
    else if (method == "setKp") {
      float val = doc["params"];
      arduinoSerial.print("SET_KP:"); arduinoSerial.println(val, 2);
    } 
    else if (method == "setKi") {
      float val = doc["params"];
      arduinoSerial.print("SET_KI:"); arduinoSerial.println(val, 2);
    } 
    else if (method == "setKd") {
      float val = doc["params"];
      arduinoSerial.print("SET_KD:"); arduinoSerial.println(val, 2);
    } 
    else if (method == "setMode") {
      String val = doc["params"]; // "AUTO" o "MANUAL"
      arduinoSerial.print("SET_MODE:"); arduinoSerial.println(val);
    }

    // Responder de inmediato para anular cartel de timeout en ThingsBoard
    String topicStr = String(topic);
    topicStr.replace("request", "response");
    client.publish(topicStr.c_str(), "{\"status\":\"ok\"}"); 
  }
}

void conectarWiFi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
}

void reconectarMQTT() {
  while (!client.connected()) {
    if (client.connect(token, token, NULL)) {
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      delay(5000);
    }
  }
}