#include <IRdevice.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
//#define DEBUG

IRdevice::IRdevice(){}


// Initialize the object created.
//
// Args:
//   deviceName: The name you will use to call the device.
//   protocol: The IR protocol used by your device. 
//             Find it yourself or decoding it as shown here: https://github.com/markszabo/IRremoteESP8266/blob/master/examples/IRrecvDumpV2/IRrecvDumpV2.ino.
//   nbits: The number of bits sent during IR transmission. 
//          Find it adding 'pre_data_bits' to 'bits' here: https://sourceforge.net/p/lirc-remotes/code/ci/master/tree/remotes/.
//   nreps: The number of times the IR signal is sent. Set it to 0 and increment it if your device needs a little bit of help.

void IRdevice::begin(const char* deviceName, const int protocol, uint16_t nbits, uint16_t nreps ){
  this->prot = protocol;
  this->bits = nbits;
  this->reps = nreps;
  this->deviceID = deviceName;
}



// Add a command to your device. The default max commands number is 5 for every device.
// Increment it in header file if you need more
//
// Args:
//   commandName: The friendly name you will use to refer to the command.
//   data: The data to be sent when the command is called. 
//         Find it the right one for your device remembering to add '0x' and 'pre_data' here: https://sourceforge.net/p/lirc-remotes/code/ci/master/tree/remotes/.

void IRdevice::addCommand(const char* commandName, uint64_t data){
  this->deviceCommands[command] = data;
  this->deviceCommandNames[command] = commandName;
  this->command++;      
}



// Check if new commands have been sent to the device. If yes, find the associated IR code.
//
// Args:
//   serverUsername: The username you have chosen for your flask server during registration.
//   deviceKey: The key the flask server has generated for you during registration.
//   serverURI: The flask server complete URI (with '/device'). 
// Returns:
//   The IR command data requested or 0 if nothing is requested.

uint64_t IRdevice::askForCommand(const char* serverUsername, const char* deviceKey, String serverURI){
  //Request body preparation
  StaticJsonBuffer<200> JSONBuffer;  //Increase the dimension if necessary
  JsonObject& JSONrequest = JSONBuffer.createObject();
  JSONrequest.set("user", serverUsername);
  JSONrequest.set("key", deviceKey);
  JSONrequest.set("type", "IR");	//for future uses
  JSONrequest.set("device", this->deviceID);
  String JSONmessageBuffer;
  JSONrequest.printTo(JSONmessageBuffer);
  
  //Communication with server
  HTTPClient http;    //Declare object of class HTTPClient
  http.begin(serverURI);      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header
  int httpCode = http.POST(JSONmessageBuffer);   //Send the request
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
  uint64_t commandData = 0;
  if (httpCode > 0){
    if (response.compareTo("None")){
      char response_c []= "";
      response.toCharArray(response_c, 50);
      Serial.print("response_c1: ");
      Serial.println(response_c);
      #ifdef DEBUG
      Serial.print("response_c2: ");
      Serial.println(response_c);
      #endif
      int comparison;
      for(int i = 0; i < this->command; i++){
        comparison = strcasecmp(this->deviceCommandNames[i], response_c);   //Case insensitive
        #ifdef DEBUG
        Serial.print("comparison: ");
        Serial.println(comparison);
        Serial.print("member1: ");
        Serial.println(this->deviceCommandNames[i]);
        #endif
        if(comparison == 0){
          Serial.println("match!");
          commandData = this->deviceCommands[i];
          break;
        }    
      }
    }
  }
  return commandData;
}



// Send IR commands to the device.
//
// Args:
//   LEDPin: The pin the IR led is connected to. Pin 4 (D2) suggested.
//   data: IR command data returned by 'AskForCommand'. 

void IRdevice::sendIR(uint16_t LEDPin, uint64_t data){
  IRsend irsend(LEDPin);
  irsend.begin();
  switch (prot) {
    case NEC: irsend.sendNEC(data, bits, reps); break;
    case SONY: irsend.sendSony(data, bits, reps); break;
    case RC5: irsend.sendRC5(data, bits, reps); break;
    case RC6: irsend.sendRC6(data, bits, reps); break;
    case DISH: irsend.sendDISH(data, bits, reps); break;
    case JVC: irsend.sendJVC(data, bits, reps); break;
    case SAMSUNG: irsend.sendSAMSUNG(data, bits, reps); break;
    case LG: irsend.sendLG(data, bits, reps); break;
    case WHYNTER: irsend.sendWhynter(data, bits, reps); break;
    case COOLIX: irsend.sendCOOLIX(data, bits, reps); break;
    case DENON: irsend.sendDenon(data, bits, reps); break;
    case SHERWOOD: irsend.sendSherwood(data, bits, reps); break;
    case RCMM: irsend.sendRCMM(data, bits, reps); break;
    case MITSUBISHI: irsend.sendMitsubishi(data, bits, reps); break;
    case SANYO: irsend.sendSanyoLC7461(data, bits, reps); break;
    case NIKAI: irsend.sendNikai(data, bits, reps); break;
    case PANASONIC: irsend.sendPanasonic64(data, bits, reps); break;
    case MAGIQUEST: irsend.sendMagiQuest(data, bits, reps); break;
    case LASERTAG: irsend.sendLasertag(data, bits, reps); break;
    case MITSUBISHI2: irsend.sendMitsubishi2(data, bits, reps); break;
    case CARRIER_AC: irsend.sendCarrierAC(data, bits, reps); break;
    case SHARP: irsend.sendSharpRaw(data, bits, reps); break;
    case AIWA_RC_T501: irsend.sendAiwaRCT501(data, bits, reps); break;
    case MIDEA: irsend.sendMidea(data, bits, reps); break;
    case GICABLE: irsend.sendGICable(data, bits, reps); break;
  }
}