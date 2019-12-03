#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include "./config.h"

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;
PubSubClient client("172.20.10.3", 1883, wifiClient);
String device_name = "esp8266-distance-sensor";
const char* ssid1 = HOME_WIFI_SSID;
const char* passwd1 = HOME_WIFI_PASSWORD;
const char* ssid2 = MATT_HOTSPOT_SSID;
const char* passwd2 = MATT_HOTSPOT_PASSWORD;

const int trigPin = D7;
const int echoPin = D6;

long distance, duration;
unsigned long timer_pointer = 0;

void connectToWifi(){
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid1, passwd1);
  wifiMulti.addAP(ssid2, passwd2);
  
  Serial.print("\nConnecting");
  
  while (wifiMulti.run() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected to ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP Address: ");
  Serial.println(WiFi.localIP());

  if (wifiMulti.run() == WL_CONNECTED){
    digitalWrite(BUILTIN_LED, LOW);
  }
}

void reconnectToHub() {
  while (!client.connected()){
    if (client.connect((char*) device_name.c_str())){
      Serial.println("Connected to MQTT Hub");
      String message = "Distance sensor (" + device_name + ") connected @ " + WiFi.localIP().toString();
      client.publish("/connections/distance", message.c_str());
    } else {
      Serial.println("Connection to MQTT failed, trying again...");
      delay(3000);
    }  
  }  
}

void setup(){
  Serial.begin(115200);
  connectToWifi();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop(){
  if (!client.connected()){
    reconnectToHub();
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  client.loop();
  if (millis() > timer_pointer + 1000){
    timer_pointer = millis();
    client.publish("/distance/p", String(distance).c_str());
  }
}
