#include <Arduino.h>
#include <LightDevice.h>
#include <ArduinoJson.h>  //You have to install it.
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>  //You have to install it.
//#define DEBUG

LightDevice::LightDevice(){}


// Initialize the object created.
//
// Args:
//   deviceName: The name you will use to call the device.
//   relayPin: The pin your relay is connected to.
// Optional (don't set them if you only want to control a bulb ON/OFF using a relay):
//   ledSetup: Your LED strip configuration. 
//             Please refer to https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino. 
//   nleds: The number of leds your leds in your strip.
//   dataPin: The pin you led strip data pin is connected to.

void LightDevice::begin(const char* deviceName, const int relayPin){
  this->begin(deviceName, relayPin, 0, 0, 0);
}

void LightDevice::begin(const char* deviceName, const int relayPin, const int ledSetup, uint16_t nleds, uint16_t dataPin){
  this->deviceID = deviceName;
  this->relay = relayPin;
  this->ledsNumber = nleds;
  if (nleds != 0){
    this->ledStrip = new Adafruit_NeoPixel(nleds, dataPin, ledSetup);
    this->ledStrip->begin();
    this->setAll(255, 255, 255);
    this->lastColor = WHITE;
    this->lastCommand = ON;
  }  
}



// Define a command for your device. The possible commands are: ON,OFF,BRIGHTNESS,COLOR,ANIMATION
//
// Args:
//   lightCommandName: The friendly name for the command without its value.
//   lightCommand: The command associated to the friendly name. 

void LightDevice::defineCommand(const char* lightCommandName, commandType lightCommand){
  this->lightCommandNames[command] = lightCommandName;
  this->lightCommands[command] = lightCommand;
  this->command++;      
}



// Define a color for your device in your language. The possible colors are: RED,GREEN,BLUE,PINK,PURPLE,CYAN,YELLOW,MAGENTA,ORANGE,INDIGO. 
//
// Args:
//   lightColorName: The friendly name for the color.
//   lightColor: The color associated to the friendly name. 

void LightDevice::defineColor(const char* lightColorName, colorType lightColor){
  this->lightColorNames[color] = lightColorName;
  this->lightColors[color] = lightColor;
  this->color++;
}



// Define an animation friendly name in your language. I've included some animations but you can add more: ANIMATION1,..,ANIMATION6.
//
// Args:
//   lightAnimationName: The friendly name for the animation.
//   lightAnimation: The animation associated to the friendly name. 

void LightDevice::defineAnimation(const char* lightAnimationName, animationType lightAnimation){
  this->lightAnimationNames[animation] = lightAnimationName;
  this->lightAnimations[animation] = lightAnimation;
  this->animation++;      
}



// Check if new commands have been sent to the device. If yes, find the associated command.
//
// Args:
//   serverUsername: The username you have chosen for your flask server during registration.
//   deviceKey: The key the flask server has generated for you during registration.
//   serverURI: The flask server complete URI (with '/device'). 
// Returns:
//   The command requested or 0 if nothing is requested/matches.

commandType LightDevice::askForCommand(const char* serverUsername, const char* deviceKey, String serverURI){
  //Request body preparation
  if (this->JSONmessageBuffer.length() == 0){
    size_t len = strlen(serverUsername);
    this->serverUsername = new char[len+1];
    strncpy(this->serverUsername, serverUsername, len+1);
    len = strlen(deviceKey);
    this->deviceKey = new char[len+1];
    strncpy(this->deviceKey, deviceKey, len+1);
    len = serverURI.length();
    this->serverURI = new char[len+1];
    serverURI.toCharArray(this->serverURI, len+1);    
    StaticJsonBuffer<200> JSONBuffer;  //Increase the dimension if necessary
    JsonObject& JSONrequest = JSONBuffer.createObject();
    JSONrequest.set("user", serverUsername);
    JSONrequest.set("key", deviceKey);
    JSONrequest.set("type", "light");	//for future uses
    JSONrequest.set("device", this->deviceID);
    JSONrequest.printTo(this->JSONmessageBuffer);
  }
  
  //Communication with server
  HTTPClient http;    //Declare object of class HTTPClient
  http.begin(serverURI);      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header
  int httpCode = http.POST(this->JSONmessageBuffer);   //Send the request
  String response = http.getString();   //Get the response payload
  #ifdef DEBUG
  Serial.print("JSONmessageBuffer: ");
  Serial.println(JSONmessageBuffer);
  Serial.print("httpCode:");
  Serial.println(httpCode);  
  Serial.print("response: ");
  Serial.println(response);
  #endif
  http.end();  //Close connection  
  
  //Server response elaboration
  commandType commandMatch = NONE;
  if (httpCode > 0){
    if (response.compareTo("None")){
      char response_c [response.length()+1];
      response.toCharArray(response_c, response.length()+1);
      const char* commandText = strtok(response_c, " ");  //Split response
      const char* token = strtok(NULL, "\0"); //into two parts
      if (token != NULL){
        if (this->requestValue){
          delete[] this->requestValue;
        }
        size_t len = strlen(token);
        this->requestValue = new char[len+1];
        strncpy(this->requestValue, token, len+1);
      }
      #ifdef DEBUG
      Serial.print("commandText: ");
      Serial.println(commandText);
      Serial.print("requestValue: ");
      Serial.println(this->requestValue);
      #endif
      int commandComparison;
      for (int i = 0; i < this->command; i++){  //Check if any defined command matches
        commandComparison = strcasecmp(this->lightCommandNames[i], commandText);   //Case insensitive
        #ifdef DEBUG
        Serial.print("commandComparison ");
        Serial.print(commandComparison?"False":"True");
        Serial.print(" with ");
        Serial.println(this->lightCommandNames[i]);
        #endif
        if(commandComparison == 0){
          Serial.println("command match!");
          commandMatch = this->lightCommands[i];
          break;
        }    
      }
    }
  }
  return commandMatch;
}



// Used inside LightDevice::setLight function to make animations run continuously until a command is received.
//
// Args:
//   LightDevice::*fn: the animation function.
//   arg: the animation function argument.

void LightDevice::animationWrapper(void (LightDevice::*fn)(colorType), colorType arg){
  long startTimeA = millis();
  this->setAll(0, 0, 0);
  while (true){
    yield();
    (this->*fn)(arg);
    if ((millis() - startTimeA) > 5000){
      startTimeA = millis();
      commandType response = (this->askForCommand(this->serverUsername, this->deviceKey, this->serverURI));
      if (response != NONE){
        this->setLight(response);
        break;
      }
    }
  }
}

void LightDevice::animationWrapper(void (LightDevice::*fn)()){
  long startTimeA = millis();
  this->setAll(0, 0, 0);
  while (true){
    yield();
    (this->*fn)();
    if ((millis() - startTimeA) > 5000){
      startTimeA = millis();
      commandType response = (this->askForCommand(this->serverUsername, this->deviceKey, this->serverURI));
      if (response != NONE){
        this->setLight(response);
        break;
      }
    }
  }
}




// Set light according to the command received.
//
// Args:
//   command: command returned from 'AskForCommand'. 

void LightDevice::setLight(commandType command){
  switch (command) {
    case ON:
      digitalWrite(this->relay, 1);
      this->setAll(255, 255, 255);
      break;
    case OFF:
      this->setAll(0, 0, 0);
      digitalWrite(this->relay, 0); 
      break;
    case BRIGHTNESS:
      if (!this->ledsNumber){
        Serial.println("Command not allowed");
      }else{  //Set brightness to the selected value and repeat previous action with the new brightness
        int brightness = atoi(requestValue);
        #ifdef DEBUG
        Serial.print("Brightness set: ");
        Serial.println(brightness);
        #endif
        this->ledStrip->setBrightness((brightness*255)/100);
        if (lastValue != NULL){
          if (this->requestValue){
            delete[] this->requestValue;
          }
          size_t len = strlen(this->lastValue);
          this->requestValue = new char[len+1];
          strncpy(this->requestValue, this->lastValue, len+1);
        }
        this->setLight(this->lastCommand);
      }
      break;
    case COLOR: 
      if (!this->ledsNumber){
        Serial.println("Command not allowed");
      }else{  //Set led color to the color selected
        this->lastCommand = COLOR;
        if (this->lastValue){
          delete[] this->lastValue;
        }
        size_t len = strlen(this->requestValue);
        this->lastValue = new char[len+1];
        strncpy(this->lastValue, this->requestValue, len+1);
        colorType colorMatch = UNDEFINED_C;
        int colorComparison, index;
        for (index = 0; index < this->color; index++){
          colorComparison = strcasecmp(this->lightColorNames[index], this->requestValue);   //Case insensitive
          #ifdef DEBUG
          Serial.print("colorComparison ");
          Serial.print(colorComparison?"False":"True");
          Serial.print(" with ");
          Serial.println(this->lightColorNames[index]);
          #endif
          if (colorComparison == 0){
            Serial.println("color match!");
            colorMatch = this->lightColors[index];
            break;
          }
        }
        this->lastColor = colorMatch;
        for (index = 0; index < this->ledsNumber; index++){
          this->ledStrip->setPixelColor(index, colorMatch);
        }
        this->ledStrip->show();
      }
      break;
    case ANIMATION:
      if (!this->ledsNumber){
        Serial.println("Command not allowed");
      }else{
        this->lastCommand = ANIMATION;
        if (this->lastValue){
          delete[] this->lastValue;
        }
        size_t len = strlen(this->requestValue);
        this->lastValue = new char[len+1];
        strncpy(this->lastValue, this->requestValue, len+1);
        animationType animationMatch = UNDEFINED_A;
        int animationComparison, index;
        for (index = 0; index < this->animation; index++){
          animationComparison = strcasecmp(this->lightAnimationNames[index], this->requestValue);   //Case insensitive
          #ifdef DEBUG
          Serial.print("animationComparison ");
          Serial.println(animationComparison?"False":"True");
          Serial.print(" with ");
          Serial.println(this->lightAnimationNames[index]);
          #endif
          if (animationComparison == 0){
            Serial.println("animation match!");
            animationMatch = this->lightAnimations[index];
            if (animationMatch == RANDOM){
              animationMatch = (animationType)random(1, this->animation);
            }
            break;
          }
        }
        switch (animationMatch) {
          case ANIMATION1:{
            this->animationWrapper(&LightDevice::animation1, this->lastColor);
          }
          break;
          case ANIMATION2:{ 
            this->animationWrapper(&LightDevice::animation2, this->lastColor);
          }
          break;
          case ANIMATION3:{
            this->animationWrapper(&LightDevice::animation3, this->lastColor);
          }
          break;
          case ANIMATION4:{
            this->animationWrapper(&LightDevice::animation4, this->lastColor);
          }
          break;
          case ANIMATION5:{
            this->animationWrapper(&LightDevice::animation5);
          }
          break;
          case ANIMATION6:{
            this->animationWrapper(&LightDevice::animation6, this->lastColor);
          }
          break;
          case ANIMATION7:{
            this->animationWrapper(&LightDevice::animation7, this->lastColor);
          }
          break;
          case ANIMATION8:{
            this->animationWrapper(&LightDevice::animation8, this->lastColor);
          }
          break;
          case ANIMATION9:{
            this->animationWrapper(&LightDevice::animation9);
          }
          break;
          case ANIMATION10:{
            this->animationWrapper(&LightDevice::animation10, this->lastColor);
          }
          break;
          case ANIMATION11:{
            this->animationWrapper(&LightDevice::animation11);
          }
          break;
          case ANIMATION12:{
            this->animationWrapper(&LightDevice::animation12, this->lastColor);
          }
          break;
          case ANIMATION13:{
            this->animationWrapper(&LightDevice::animation13);
          }
          break;
          case UNDEFINED_A: ; break;
        }
      }
      break;
    case NONE:
    break;
  }
}

void LightDevice::setAll(uint8_t red, uint8_t green, uint8_t blue) {
  for(int i = 0; i < this->ledsNumber; i++ ) {
    this->ledStrip->setPixelColor(i, red, green, blue); 
  }
  this->ledStrip->show();
}

void LightDevice::animation1(colorType packedColor){ //FadeInOut
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  float r, g, b;
  for(int k = 0; k < 256; k=k+1) { 
    delay(0);
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    this->setAll(r,g,b);
    this->ledStrip->show();
  }
  for(int k = 255; k >= 0; k=k-2) {
    delay(0);
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    this->setAll(r,g,b);
    this->ledStrip->show();
  }
}

void LightDevice::animation2(colorType packedColor){  //Strobe
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  for(int j = 0; j < 10; j++) {
    this->setAll(red,green,blue);
    this->ledStrip->show();
    delay(50);
    setAll(0,0,0);
    this->ledStrip->show();
    delay(50);
  }
}

void LightDevice::animation3(colorType packedColor){  //Cylon
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  for(int i = 0; i < this->ledsNumber-4-2; i++) {
    this->setAll(0,0,0);
    this->ledStrip->setPixelColor(i, red/10, green/10, blue/10);
    for(int j = 1; j <= 4; j++) {
      this->ledStrip->setPixelColor(i+j, red, green, blue);
    }
    this->ledStrip->setPixelColor(i+4+1, red/10, green/10, blue/10);
    this->ledStrip->show();;
    delay(10);
  }
  delay(50);
  for(int i = this->ledsNumber-4-2; i > 0; i--) {
    setAll(0,0,0);
    this->ledStrip->setPixelColor(i, red/10, green/10, blue/10);
    for(int j = 1; j <= 4; j++) {
      this->ledStrip->setPixelColor(i+j, red, green, blue);
    }
    this->ledStrip->setPixelColor(i+4+1, red/10, green/10, blue/10);
    this->ledStrip->show();
    delay(10);
  }
  delay(50);
}

void LightDevice::animation4(colorType packedColor){  //Twinkle
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  this->setAll(0,0,0);
  for (int i=0; i<10; i++) {
     this->ledStrip->setPixelColor(random(this->ledsNumber),red,green,blue);
     this->ledStrip->show();;
     delay(100);
   }
  delay(100);
}

void LightDevice::animation5(){  //Twinkle random color
  randomSeed(analogRead(0));
  uint8_t red = random(255);
  randomSeed(analogRead(0));
  uint8_t green = random(255);
  randomSeed(analogRead(0));
  uint8_t blue = random(255);
  this->setAll(0,0,0);
  for (int i=0; i<20; i++) {
     this->ledStrip->setPixelColor(random(this->ledsNumber),red,green,blue);
     this->ledStrip->show();
     delay(100);
   }
  delay(100);
}

void LightDevice::animation6(colorType packedColor){  //Sparkle
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  int Pixel = random(this->ledsNumber);
  this->ledStrip->setPixelColor(Pixel,red,green,blue);
  this->ledStrip->show();
  this->ledStrip->setPixelColor(Pixel,0,0,0);
}

void LightDevice::animation7(colorType packedColor){  //Running Lights
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  int Position=0;
  for(int j=0; j<this->ledsNumber*2; j++){
    Position++;
    for(int i=0; i<this->ledsNumber; i++) {
      this->ledStrip->setPixelColor(i,((sin(i+Position) * 127 + 128)/255)*red,
                  ((sin(i+Position) * 127 + 128)/255)*green,
                  ((sin(i+Position) * 127 + 128)/255)*blue);

    }   
    this->ledStrip->show();
    delay(50);
  }
}

void LightDevice::animation8(colorType packedColor){  //Color Wipe
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  for(uint16_t i=0; i<this->ledsNumber; i++) {
    this->ledStrip->setPixelColor(i, red, green, blue);
    this->ledStrip->show();
    delay(30);
  }
  for(uint16_t i=0; i<this->ledsNumber; i++) {
    this->ledStrip->setPixelColor(i, 0, 0, 0);
    this->ledStrip->show();
  }
}

byte * LightDevice::Wheel(byte WheelPos) {
  static byte c[3];
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}

void LightDevice::animation9(){  //Rainbow Cycle
  byte *c;
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i<this->ledsNumber; i++) {
      c=this->Wheel(((i * 256 / this->ledsNumber) + j) & 255);
      this->ledStrip->setPixelColor(i, *c, *(c+1), *(c+2));
    }
    this->ledStrip->show();
    delay(20);
  }
}

void LightDevice::animation10(colorType packedColor){  //Theatre Chase
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  for (int j=0; j<10; j++) {
    for (int q=0; q < 3; q++) {
      for (int i=0; i < this->ledsNumber; i=i+3) {
        this->ledStrip->setPixelColor(i+q, red, green, blue);
      }
      this->ledStrip->show();
      delay(50);
      for (int i=0; i < this->ledsNumber; i=i+3) {
        this->ledStrip->setPixelColor(i+q, 0,0,0);
      }
    }
  }
}

void LightDevice::animation11(){  //Theatre Chase Rainbow
  byte *c;
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < ledsNumber; i=i+3) {
        c = this->Wheel( (i+j) % 255);
        this->ledStrip->setPixelColor(i+q, *c, *(c+1), *(c+2));    //turn every third pixel on
      }
      this->ledStrip->show();
      delay(50);
      for (int i=0; i < this->ledsNumber; i=i+3) {
        this->ledStrip->setPixelColor(i+q, 0,0,0);        //turn every third pixel off
      }
    }
  }
}

void LightDevice::animation12(colorType packedColor){  //Bouncing Balls
  uint8_t red = packedColor >> 16;
  uint8_t green = (packedColor&16777215) >> 8;
  uint8_t blue = packedColor&255;
  int boh = this->ledsNumber * 30;
  int BallCount = (int)(1+(this->ledsNumber/30));
  float Gravity = -9.81;
  int StartHeight = 1;
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
  for (int i = 0 ; i < BallCount ; i++) {
    delay(0);   
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0; 
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2); 
  }
  while (boh>0) {
    delay(0);
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (this->ledsNumber - 1) / StartHeight);
    }
    for (int i = 0 ; i < BallCount ; i++) {
      this->ledStrip->setPixelColor(Position[i],red,green,blue);
    }
    this->ledStrip->show();
    this->setAll(0,0,0);
    boh--;
  }
}

void LightDevice::animation13(){  //Multi Color Bouncing Balls
  randomSeed(analogRead(0));
  uint8_t red = random(255);
  randomSeed(analogRead(0));
  uint8_t green = random(255);
  randomSeed(analogRead(0));
  uint8_t blue = random(255);
  int boh = this->ledsNumber * 30;
  int BallCount = (int)(1+(this->ledsNumber/30));
  float Gravity = -9.81;
  int StartHeight = 1;
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
  for (int i = 0 ; i < BallCount ; i++) {  
    delay(0); 
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0; 
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2); 
  }
  while (boh>0) {
    delay(0);
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (this->ledsNumber - 1) / StartHeight);
    }
    for (int i = 0 ; i < BallCount ; i++) {
      this->ledStrip->setPixelColor(Position[i],red,green,blue);
    }
    this->ledStrip->show();
    this->setAll(0,0,0);
    boh--;
  }
}

void LightDevice::fadeToBlack(int ledNo, byte fadeValue) {
  uint32_t oldColor;
  uint8_t r, g, b;
  //int value;
  oldColor = this->ledStrip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);
  r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
  g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
  b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
  this->ledStrip->setPixelColor(ledNo, r,g,b);
}
