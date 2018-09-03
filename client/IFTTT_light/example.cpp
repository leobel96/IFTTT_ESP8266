#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <LightDevice.h>

#include "credentials.h"

const char* wifiSSID = WIFI_SSID; //Your WiFi SSID
const char* wifiPass = WIFI_PASSWORD;  //Your WiFi password

String serverURI = SERVER_URI; //Your flask server URI with "/device"
const char* serverUsername = SERVER_USERNAME;  //Your username chosen during registration
const char* deviceKey = DEVICE_KEY; //The key retrieved during registration or the refreshed one

LightDevice luce1;  //Object initialization

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
  luce1.begin("5", 2, NEO_GRB + NEO_KHZ800, 64, 4); //(deviceName, relayPin, ledSetup, nleds, dataPin)
  
  luce1.defineCommand("accendi", ON); //(commandName, command)
  luce1.defineCommand("spegni", OFF);
  luce1.defineCommand("luminosità", BRIGHTNESS);
  luce1.defineCommand("colore", COLOR);
  luce1.defineCommand("animazione", ANIMATION);
  
  luce1.defineColor("rosso", RED);  //(colorName, color)
  luce1.defineColor("verde", GREEN);
  luce1.defineColor("blu", BLUE);
  luce1.defineColor("viola", PURPLE);
  luce1.defineColor("rosa", PINK);
  luce1.defineColor("indaco", INDIGO);
  luce1.defineColor("arancione", ORANGE);
  luce1.defineColor("giallo", YELLOW);
  luce1.defineColor("ciano", CYAN);
  luce1.defineColor("magenta", MAGENTA);

  luce1.defineAnimation("casuale", RANDOM); //(animationName, animation)
  luce1.defineAnimation("1", ANIMATION1);
  luce1.defineAnimation("2", ANIMATION2);  
  luce1.defineAnimation("3", ANIMATION3);
  luce1.defineAnimation("4", ANIMATION4);  
  luce1.defineAnimation("5", ANIMATION5);  
  luce1.defineAnimation("6", ANIMATION6);
  luce1.defineAnimation("7", ANIMATION7);  
  luce1.defineAnimation("8", ANIMATION8);  
  luce1.defineAnimation("9", ANIMATION9);
  luce1.defineAnimation("10", ANIMATION10);  
  luce1.defineAnimation("11", ANIMATION11);  
  luce1.defineAnimation("12", ANIMATION12);
  luce1.defineAnimation("13", ANIMATION13);  
}

void loop() {
  commandType response = luce1.askForCommand(serverUsername, deviceKey, serverURI);
  luce1.setLight(response);
}
	

