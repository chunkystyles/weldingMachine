#include <Arduino.h>
#include <TM1637Display.h>
#include <EEPROM.h>

enum MACHINE_STATE {
  STOP,
  RUN,
  ADJUST
};

enum TOGGLE {
  FIRST_SPD,
  SECOND_SPD,
  FIRST_DEGREES,
  SECOND_DEGREES
};

//Pins
#define adjustPotPin       0
#define adjustButtonPin    2
#define startStopButtonPin 3
#define toggleButtonPin    4
#define displayClkPin      5
#define displayDioPin      6

#define driverPulPin       11
#define driverDirPin       12
#define driverEnaPin       13

//Configuration
#define potMaxValue 1023
#define potAdjustment 0.20
#define adjustDelay 100
#define toggleDelay 300
#define toggleTimeout 2000

const float adjustIncrement = potAdjustment / (float) potMaxValue;

//Global variables
int previousPotValue = 0;
int initialPotValue = 0;
MACHINE_STATE machineState = STOP;
MACHINE_STATE previousMachineState = STOP;
TOGGLE toggle = FIRST_SPD;
long toggleStart = 0;
TM1637Display display(displayClkPin, displayDioPin);
 
void setup(){
  pinMode(adjustButtonPin, INPUT_PULLUP);
  pinMode(startStopButtonPin, INPUT_PULLUP);
  pinMode(driverPulPin, OUTPUT);
  pinMode(driverDirPin, OUTPUT);
  pinMode(driverEnaPin, OUTPUT);
  pinMode(toggleButtonPin, INPUT_PULLUP);
  display.setBrightness(0x0f);
  readStepperValuesFromEeprom();
  calculateStepperValues();
  displayMachineState();
}
 
void loop(){
  handleInput();
  advanceToggleTimeout();
  if (previousMachineState != machineState){
    displayMachineState();
    previousMachineState = machineState;
  }
  handleStepper();
}

int roundUp(float decimal){
  return (int)(decimal + 0.5);
}
