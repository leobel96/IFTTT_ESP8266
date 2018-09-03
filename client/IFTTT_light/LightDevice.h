#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MAX_COMMANDS_NUMBER 6
#define MAX_COLORS_NUMBER 13
#define MAX_ANIMATIONS_NUMBER 16


//Colors packed using Adafruit_NeoPixel::Color method
enum colorType : uint32_t{
  UNDEFINED_C = 0,
  RED = 16711680,
  GREEN = 65280,
  BLUE = 255,
  INDIGO = 4915330,
  YELLOW = 16776960,
  CYAN = 65535,
  MAGENTA = 16711935,
  ORANGE = 16753920,
  PURPLE = 8388736,
  PINK = 16761035,
  BLACK = 0,
  WHITE = 16777215
};


//Possible commands
enum commandType {
  NONE = 0,
  ON,
  OFF,
  BRIGHTNESS,
  COLOR,
  ANIMATION
};


enum animationType {
  RANDOM,
  UNDEFINED_A = 0,
  ANIMATION1 = 1,
  ANIMATION2 = 2,
  ANIMATION3 = 3,
  ANIMATION4 = 4,
  ANIMATION5 = 5,
  ANIMATION6 = 6,
  ANIMATION7 = 7,
  ANIMATION8 = 8,
  ANIMATION9 = 9,
  ANIMATION10 = 10,
  ANIMATION11 = 11,
  ANIMATION12 = 12,
  ANIMATION13 = 13,
};



class LightDevice{
public:
  //Begin
  uint8_t command = 0;
  uint8_t color = 0;
  uint8_t animation = 0;
  
  const char* deviceID;
  int relay;
  uint16_t ledsNumber;
  Adafruit_NeoPixel *ledStrip;

  //Command, color and animation definitions
  commandType lightCommands[MAX_COMMANDS_NUMBER];
  const char* lightCommandNames[MAX_COMMANDS_NUMBER];
  colorType lightColors[MAX_COLORS_NUMBER];
  const char* lightColorNames[MAX_COLORS_NUMBER];
  animationType lightAnimations[MAX_ANIMATIONS_NUMBER];
  const char* lightAnimationNames[MAX_ANIMATIONS_NUMBER];
  
  String JSONmessageBuffer;
  char* requestValue;
  char* serverUsername;
  char* deviceKey;
  char* serverURI;
  
  commandType lastCommand;
  char* lastValue;
  colorType lastColor;

  //Class Methods
  LightDevice();
  void begin(const char* deviceName, const int relayPin);
  void begin(const char* deviceName, int relayPin, const int ledSetup, uint16_t nleds, uint16_t dataPin);
  void defineCommand(const char* lightCommandName, commandType lightCommand);
  void defineColor(const char* lightColorName, colorType lightColor);
  void defineAnimation(const char* lightAnimationName, animationType lightAnimation);
  commandType askForCommand(const char* serverUsername, const char* deviceKey, String serverURI);
  void animationWrapper(void (LightDevice::*fn)(colorType), colorType a);
  void animationWrapper(void (LightDevice::*fn)());
  void setLight(commandType command);

  void setAll(uint8_t red, uint8_t green, uint8_t blue);
  void fadeToBlack(int ledNo, byte fadeValue);
  byte * Wheel(byte WheelPos);
  byte * Wheel_b(byte WheelPos);
  
  //Animations
  void animation1(colorType packedColor);
  void animation2(colorType packedColor);
  void animation3(colorType packedColor);
  void animation4(colorType packedColor);
  void animation5();
  void animation6(colorType packedColor);
  void animation7(colorType packedColor);
  void animation8(colorType packedColor);
  void animation9();
  void animation10(colorType packedColor);
  void animation11();
  void animation12(colorType packedColor);
  void animation13();
  //Add here your animation declaration
};