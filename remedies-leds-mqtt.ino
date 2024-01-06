#include "variables.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>

HTTPClient http;
WiFiClient espClient;
PubSubClient client(espClient);

// Servos init
const int pinLedMondayA = 12;
const int pinLedMondayB = 13;

const int pinLedTuesdayA = 14;
const int pinLedTuesdayB = 27;

const int pinLedWednesdayA = 26;
const int pinLedWednesdayB = 23; //25

const int pinLedThursdayA = 33;
const int pinLedThursdayB = 32;

const int pinLedFridayA = 16;
const int pinLedFridayB = 17;

const int pinLedSaturdayA = 18;
const int pinLedSaturdayB = 19;

const int pinLedSundayA = 21;
const int pinLedSundayB = 22;

const int pinLedWifi = 4;
const int pinLedMqtt = 5;

void setup() {
  Serial.begin(9600);

  pinMode(pinLedWifi, OUTPUT);
  pinMode(pinLedMqtt, OUTPUT);

  connectWifi();  

  connectMqtt(); 

  attachLeds();

  getStatus();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void connectWifi() {
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(1000);
     digitalWrite(pinLedMqtt, LOW);
     digitalWrite(pinLedWifi, LOW);
     Serial.println("Connecting to WiFi..");
  }
  digitalWrite(pinLedWifi, HIGH);
  
  Serial.println("Connected to the WiFi network");
}

void connectMqtt() {
    //connecting to a mqtt broker
   client.setServer(mqtt_broker, mqtt_port);
   client.setCallback(callback);
   while (!client.connected()) {
       String client_id = "esp32-client-";
       client_id += String(WiFi.macAddress());
       Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
       if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          digitalWrite(pinLedMqtt, HIGH);
          //updateConnectionOnline(true);
          Serial.println("Public emqx mqtt broker connected");
       } else {
          digitalWrite(pinLedMqtt, LOW);
          //updateConnectionOnline(false);
          Serial.println("failed with state ");
          Serial.print(client.state());
          delay(5000);
       }
   }
   // publish and subscribe
   //client.publish(topic, "Hi EMQX I'm ESP32 ^^");
   client.subscribe(topic);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando reconectarse MQTT...");
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      digitalWrite(pinLedMqtt, HIGH);
      //updateConnectionOnline(true);
      Serial.println("Public emqx mqtt broker connected");
          
    } else {
      digitalWrite(pinLedMqtt, LOW);
      //updateConnectionOnline(false);
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  client.subscribe(topic);
}

void attachLeds() {
  
  pinMode(pinLedMondayA, OUTPUT);
  pinMode(pinLedMondayB, OUTPUT);

  pinMode(pinLedTuesdayA, OUTPUT);
  pinMode(pinLedTuesdayB, OUTPUT);

  pinMode(pinLedWednesdayA, OUTPUT);
  pinMode(pinLedWednesdayB, OUTPUT);

  pinMode(pinLedThursdayA, OUTPUT);
  pinMode(pinLedThursdayB, OUTPUT);

  pinMode(pinLedFridayA, OUTPUT);
  pinMode(pinLedFridayB, OUTPUT);

  pinMode(pinLedSaturdayA, OUTPUT);
  pinMode(pinLedSaturdayB, OUTPUT);

  pinMode(pinLedSundayA, OUTPUT);
  pinMode(pinLedSundayB, OUTPUT);
}

void updateConnectionOnline(bool isOnline) {
  Serial.print("HTTP actualizando online");
  http.useHTTP10(true);
  http.begin("https://" + urlAPI + "/api/auth/" + idEsp32 + "/online"); //HTTP
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-token", tokenESP32);

  String jsonPayload = isOnline ? "{\"online\": true}" : "{\"online\": false}";
  int httpResponseCode = http.PUT(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");

    // Read the response
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error in HTTP request. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


void getStatus() {
  http.useHTTP10(true);
  http.begin("https://" + urlAPI + "/api/v2/status"); //HTTP
  http.addHeader("x-token", tokenESP32);
  int httpCode = http.GET();

  if(httpCode > 0) {
    Serial.printf("[HTTP] GET. code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getString());
      JsonArray jsonArray = doc.as<JsonArray>();

      for (JsonVariant value : jsonArray) {
        String day = value["day"].as<String>();
        bool enabledAM = value["enabledAM"].as<bool>();
        bool enabledPM = value["enabledPM"].as<bool>();
        initLeds(day, enabledAM, enabledPM);
      }      
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void initLeds(String day, bool enabledAM, bool enabledPM) {
  int dayNum = getDayNumber(day);
  switch(dayNum) {
    case 1:
      enabledAM ? digitalWrite(pinLedMondayA, HIGH) : digitalWrite(pinLedMondayA, LOW);
      enabledPM ? digitalWrite(pinLedMondayB, HIGH) : digitalWrite(pinLedMondayB, LOW);
      break;
    case 2:
      enabledAM ? digitalWrite(pinLedTuesdayA, HIGH) : digitalWrite(pinLedTuesdayA, LOW);
      enabledPM ? digitalWrite(pinLedTuesdayB, HIGH) : digitalWrite(pinLedTuesdayB, LOW);
      break;
    case 3:
      enabledAM ? digitalWrite(pinLedWednesdayA, HIGH) : digitalWrite(pinLedWednesdayA, LOW);
      enabledPM ? digitalWrite(pinLedWednesdayB, HIGH) : digitalWrite(pinLedWednesdayB, LOW);
      break;
    case 4:
      enabledAM ? digitalWrite(pinLedThursdayA, HIGH) : digitalWrite(pinLedThursdayA, LOW);
      enabledPM ? digitalWrite(pinLedThursdayB, HIGH) : digitalWrite(pinLedThursdayB, LOW);
      break;
    case 5:
      enabledAM ? digitalWrite(pinLedFridayA, HIGH) : digitalWrite(pinLedFridayA, LOW);
      enabledPM ? digitalWrite(pinLedFridayB, HIGH) : digitalWrite(pinLedFridayB, LOW);
      break;
    case 6:
      enabledAM ? digitalWrite(pinLedSaturdayA, HIGH) : digitalWrite(pinLedSaturdayA, LOW);
      enabledPM ? digitalWrite(pinLedSaturdayB, HIGH) : digitalWrite(pinLedSaturdayB, LOW);
      break;
    case 7:
      enabledAM ? digitalWrite(pinLedSundayA, HIGH) : digitalWrite(pinLedSundayA, LOW);
      enabledPM ? digitalWrite(pinLedSundayB, HIGH) : digitalWrite(pinLedSundayB, LOW);
      break;
  }  
}

void changeLeds(String day, String key, bool value) {
  int dayNum = getDayNumber(day);
  switch(dayNum) {
    case 1:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedMondayA, HIGH) : digitalWrite(pinLedMondayA, LOW);
      } else{
        value ? digitalWrite(pinLedMondayB, HIGH) : digitalWrite(pinLedMondayB, LOW);
      }
      break;
    case 2:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedTuesdayA, HIGH) : digitalWrite(pinLedTuesdayA, LOW);
      } else{
        value ? digitalWrite(pinLedTuesdayB, HIGH) : digitalWrite(pinLedTuesdayB, LOW);
      }
      break;
    case 3:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedWednesdayA, HIGH) : digitalWrite(pinLedWednesdayA, LOW);
      } else{
        value ? digitalWrite(pinLedWednesdayB, HIGH) : digitalWrite(pinLedWednesdayB, LOW);
      }
      break;
    case 4:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedThursdayA, HIGH) : digitalWrite(pinLedThursdayA, LOW);
      } else{
        value ? digitalWrite(pinLedThursdayB, HIGH) : digitalWrite(pinLedThursdayB, LOW);
      }
      break;
    case 5:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedFridayA, HIGH) : digitalWrite(pinLedFridayA, LOW);
      } else{
        value ? digitalWrite(pinLedFridayB, HIGH) : digitalWrite(pinLedFridayB, LOW);
      }
      break;
    case 6:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedSaturdayA, HIGH) : digitalWrite(pinLedSaturdayA, LOW);
      } else{
        value ? digitalWrite(pinLedSaturdayB, HIGH) : digitalWrite(pinLedSaturdayB, LOW);
      }
      break;
    case 7:
      if (key == "enabledAM") {
        value ? digitalWrite(pinLedSundayA, HIGH) : digitalWrite(pinLedSundayA, LOW);
      } else{
        value ? digitalWrite(pinLedSundayB, HIGH) : digitalWrite(pinLedSundayB, LOW);
      }
      break;
  }  
}

int getDayNumber(String day) {
  if (day == "LUNES") return 1;
  if (day == "MARTES") return 2;
  if (day == "MIÉRCOLES") return 3;
  if (day == "JUEVES") return 4;
  if (day == "VIERNES") return 5;
  if (day == "SÁBADO") return 6;
  if (day == "VIERNES") return 7;
}

void callback(char *topic, byte *payload, unsigned int length) {
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);
   Serial.print("Message:");
  
   size_t payloadSize = sizeof(payload) - 1;
   DynamicJsonDocument doc(payloadSize + 256);
   DeserializationError error = deserializeJson(doc, payload);

  if (error) {
      Serial.print(F("Error al deserializar JSON: "));
      Serial.println(error.c_str());
      return;
  }

  String day = doc["day"];
  String key = doc["key"];
  bool value = doc["value"].as<bool>();

  changeLeds(day, key, value);
  Serial.println(day);
  Serial.println(key);
  Serial.println(value);
  Serial.println("-----------------------");
}
