#include <Arduino.h>

#define MAX_COMMANDS_NUMBER 5


class IRdevice{
public:
  uint16_t bits;
  uint8_t reps;
  uint16_t command = 0;
  int prot;
  const char* deviceID;
  uint64_t deviceCommands[MAX_COMMANDS_NUMBER];
  const char* deviceCommandNames[MAX_COMMANDS_NUMBER]; 

  IRdevice();
  void begin(const char* deviceName, const int protocol, uint16_t nbits, uint16_t nreps);
  void addCommand(const char* command, uint64_t data);
  uint64_t askForCommand(const char* serverUsername, const char* deviceKey, String serverURI);
  void sendIR(uint16_t LEDPin, uint64_t data);
};