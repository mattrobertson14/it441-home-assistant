#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include "./config.h"

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;
PubSubClient client("172.20.10.3", 1883, wifiClient);
String device_name = "esp8266-door-sensor";
const char* ssid1 = HOME_WIFI_SSID;
const char* passwd1 = HOME_WIFI_PASSWORD;
const char* ssid2 = MATT_HOTSPOT_SSID;
const char* passwd2 = MATT_HOTSPOT_PASSWORD;

bool prevDoor = false;
bool door = false;
int SENSOR_PIN = D6;
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
      String message = "Door sensor (" + device_name + ") connected @ " + WiFi.localIP().toString();
      client.publish("/connections/door", message.c_str());
    } else {
      Serial.println("Connection to MQTT failed, trying again...");
      delay(3000);
    }  
  }  
}

void setup(){
  Serial.begin(115200);
  connectToWifi();
  pinMode(SENSOR_PIN, INPUT_PULLUP);
}

void loop(){
  if (!client.connected()){
    reconnectToHub();
  }

  client.loop();

  if (digitalRead(SENSOR_PIN) == HIGH){
    door = true;
  } else {
    door = false;
  }

  if (prevDoor != door){
    Serial.println(door);
    client.publish("/door", (door?"1":"0"));
    prevDoor = door;
  }

  if (millis() > timer_pointer + 5000){
    timer_pointer = millis();
    client.publish("/door/p", (door?"1":"0"));
  }
}
