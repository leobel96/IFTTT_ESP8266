#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <IRdevice.h>
#include <IRremoteESP8266.h>

#include "credentials.h"

#define LEDPIN 4

const char* wifiSSID = WIFI_SSID; //Your WiFi SSID
const char* wifiPass = WIFI_PASSWORD;  //Your WiFi password

String serverURI = SERVER_URI; //Your flask server URI with "/device"
const char* serverUsername = SERVER_USERNAME;  //Your username chosen during registration
const char* deviceKey = DEVICE_KEY; //The key retrieved during registration or the refreshed one

IRdevice TV;  //Object initialization
IRdevice TV_due;

void wifiConnect(){
	WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPass);   //WiFi connection 
  Serial.println("Connecting to WiFI");
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion 
    delay(200);     
    Serial.print(".");
  }
  Serial.println("Connected");  
}

void setup() {
	Serial.begin(19200);
	wifiConnect();  //Connect to WiFi
  delay(200);
	TV.begin("5", NEC, 32, 0);  //(deviceName, protocol, nbits, nreps )
  TV_due.begin("4", NEC, 32, 0);
  TV_due.addCommand("on", 0x20DF10EF);
	TV.addCommand("on", 0x20DF10EF);  //(commandName, data)
  TV.addCommand("off", 0x20DF10EF);
  TV.addCommand("canale 5", 0x20DFA857);
  TV.addCommand("volume più", 0x20DF40BF );
}

void loop() {
  uint64_t response = TV.askForCommand(serverUsername, deviceKey, serverURI);
  uint64_t response2 = TV_due.askForCommand(serverUsername, deviceKey, serverURI);
  if (response){
    TV.sendIR(LEDPIN, response);
  }
  if (response2){
    TV_due.sendIR(LEDPIN, response2);
  }
}
	

