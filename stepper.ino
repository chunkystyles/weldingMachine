#define directionForward HIGH
#define directionBackward LOW
#define stepperEnabled LOW
#define stepperDisabled HIGH
#define degreesPerRev 360
#define maxSpeedDelay 350
#define firstSegmentDegreesStore 0
#define secondSegmentDegreesStore 8
#define firstSegmentRpmStore 16
#define secondSegmentRpmStore 24

/*
  Below values are for microstep set to 1/2 on the stepper driver
  Changing the microstep will make these values change
*/

/*
  This is the microstep setting on the stepper driver
  It is the denominator of the fraction on that setting
  If driver set to 1, microStep = 1
  If driver set to 1/2, microStep = 2, etc.
*/
const long microStep = 2;

/*
  Stepper motor = 200 steps per rev * gear ratio of 100:1 = 20,000
*/
const long stepsPerRev = 20000 * microStep;

/*
  60000000 microseconds in a minute
  divide by 2 for 2 delays = 30000000
  (2 delays because each pulse requires a delay after HIGH and after LOW signals to complete a full pulse)
  divide by 20000 steps per revolution = 1500
*/
const int oneRpmDelay = 1500 / microStep;

/*
  The below values are all store in EEPROM and retrieved from EEPROM on startup
*/
int firstSegmentDegrees;   // originally 370;
int secondSegmentDegrees;  // originally 20;
float firstSegmentRpm;     // originally 0.5;
float secondSegmentRpm;    // originally 2.0;

//First segment
float firstRevFraction;    //The number of revolutions in first segment
long firstStepNumber;      //Number of steps needed to complete first segment
int firstDelay;            //This determines the speed

//Second segment
long secondStepNumber;     //Number of steps needed to complete second segment
int secondStartDelay;      //This determines the beginning speed, starts the same as the first segment
int secondEndDelay;        //This determines the finishing speed
float secondStepIncrement; //This is how much the delay changes every pulse
float currentReduction;    //This is the number that the delay shrinks each step, causing it to speed up
float currentDelay;        //This is the delay needed for the increased speed in segment 2

long count;
long totalSteps;
long displayUpdateStep;

void calculateStepperValues(){
  firstRevFraction = (float) firstSegmentDegrees / (float) degreesPerRev;
  firstStepNumber = stepsPerRev * firstRevFraction;
  firstDelay = oneRpmDelay / firstSegmentRpm;
  secondStepNumber = stepsPerRev * ((float) secondSegmentDegrees / (float) degreesPerRev);
  secondStartDelay = firstDelay;
  secondEndDelay = oneRpmDelay / secondSegmentRpm;
  secondStepIncrement = (float)(secondStartDelay - secondEndDelay) / (float)secondStepNumber;
  totalSteps = firstStepNumber + secondStepNumber;
  displayUpdateStep = totalSteps / (timer_segments - 1);
}

void readStepperValuesFromEeprom(){
  EEPROM_readAnything(firstSegmentDegreesStore, firstSegmentDegrees);
  EEPROM_readAnything(secondSegmentDegreesStore, secondSegmentDegrees);
  EEPROM_readAnything(firstSegmentRpmStore, firstSegmentRpm);
  EEPROM_readAnything(secondSegmentRpmStore, secondSegmentRpm);
}

void handleStepper(){
  if (machineState == RUN){
    displayRun();
    digitalWrite(driverEnaPin, stepperEnabled);
    digitalWrite(driverDirPin, directionForward);
    doFirstSegment();
    doSecondSegment();
    digitalWrite(driverEnaPin, stepperDisabled);
    machineState = STOP;
  }
}

void doFirstSegment(){
  count = 0;
  while (count++ < firstStepNumber){
    sendStepperPulse(firstDelay);
  }
}

void doSecondSegment(){
  currentReduction = 0.0;
  currentDelay = secondStartDelay;
  while (count++ < totalSteps){
    currentReduction += secondStepIncrement;
    currentDelay = secondStartDelay - (int)currentReduction;
    sendStepperPulse((int)currentDelay);
  }
}

float changeFirstSegmentByPercent(float percent, float set){
  if (set){
    firstSegmentRpm *= percent;
    calculateStepperValues();
    return firstSegmentRpm;
  } else {
    return firstSegmentRpm * percent;
  }
}

float getAdjustedValue(float adjust){
  switch (toggle) {
    case FIRST_SPD:
      return (float)firstSegmentRpm * adjust;
      break;
    case SECOND_SPD:
      return (float)secondSegmentRpm * adjust;
      break;
    case FIRST_DEGREES:
      return (float)firstSegmentDegrees * adjust;
      break;
    case SECOND_DEGREES:
      return (float)secondSegmentDegrees * adjust;
      break;
    default:
      return 0.0;
      break;
  }
}

void setAdjustedValue(float adjust){
  switch (toggle) {
    case FIRST_SPD:
      firstSegmentRpm *= adjust;
      EEPROM_writeAnything(firstSegmentRpmStore, firstSegmentRpm);
      break;
    case SECOND_SPD:
      secondSegmentRpm *= adjust;
      EEPROM_writeAnything(secondSegmentRpmStore, secondSegmentRpm);
      break;
    case FIRST_DEGREES:
      firstSegmentDegrees = roundUp((float)firstSegmentDegrees * adjust);
      EEPROM_writeAnything(firstSegmentDegreesStore, firstSegmentDegrees);
      break;
    case SECOND_DEGREES:
      secondSegmentDegrees = roundUp((float)secondSegmentDegrees * adjust);
      EEPROM_writeAnything(secondSegmentDegreesStore, secondSegmentDegrees);
      break;
    default:
      break;
  }
  calculateStepperValues();
}

void sendStepperPulse(int delayMicro){
  if (delayMicro < maxSpeedDelay){
    delayMicro = maxSpeedDelay;
  }
  digitalWrite(driverPulPin, HIGH);
  delayMicroseconds(delayMicro);
  digitalWrite(driverPulPin, LOW);
  delayMicroseconds(delayMicro);
}
