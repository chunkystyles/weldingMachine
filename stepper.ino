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
#define rampTimeStore 32

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
int firstSegmentDegrees;   //How far the second segment turns
int secondSegmentDegrees;  //How far the first segment turns
float firstSegmentRpm;     //Desired first segment speed, it is constant for the first segment
float secondSegmentRpm;    //Desired second segment speed, it will ramp from the first segment speed to this
float rampTime;            //Time in seconds of how long it takes to ramp from the start speed to the end speed

//First segment
float firstRevFraction;    //The number of revolutions in first segment
long firstStepNumber;      //Number of steps needed to complete first segment
int firstDelay;            //This determines the speed

//Second segment
long secondStepNumber;     //Number of steps needed to complete second segment
int secondStartDelay;      //This determines the beginning speed, starts the same as the first segment
int secondEndDelay;        //This determines the finishing speed
float currentReduction;    //This is the number that the delay shrinks each step, causing it to speed up
float currentDelay;        //This is the delay needed for the increased speed in segment 2
long rampTimeMicro;        //Ramp time in microseconds, so multiplied by 1,000,000
long rampElapsedMicro;     //How much time has been spent ramping up, starts at zero and goes up to rampTime
float rampPercent;         //Percentage of ramp achieved, starts at zero and goes to one
float delayDifference;     //Difference between start delay and end delay

long count;
long totalSteps;

void calculateStepperValues(){
  firstRevFraction = (float) firstSegmentDegrees / (float) degreesPerRev;
  firstStepNumber = stepsPerRev * firstRevFraction;
  firstDelay = oneRpmDelay / firstSegmentRpm;
  secondStepNumber = stepsPerRev * ((float) secondSegmentDegrees / (float) degreesPerRev);
  totalSteps = firstStepNumber + secondStepNumber;
  secondStartDelay = firstDelay;
  secondEndDelay = oneRpmDelay / secondSegmentRpm;
  delayDifference = secondStartDelay - secondEndDelay;
  rampTimeMicro = rampTime * 1000000;
}

void readStepperValuesFromEeprom(){
  EEPROM_readAnything(firstSegmentDegreesStore, firstSegmentDegrees);
  EEPROM_readAnything(secondSegmentDegreesStore, secondSegmentDegrees);
  EEPROM_readAnything(firstSegmentRpmStore, firstSegmentRpm);
  EEPROM_readAnything(secondSegmentRpmStore, secondSegmentRpm);
  EEPROM_readAnything(rampTimeStore, rampTime);
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
  currentDelay = secondStartDelay;
  rampElapsedMicro = 0;
  while (count++ < totalSteps){
    if (rampElapsedMicro <= rampTimeMicro){
      rampPercent = (float)rampElapsedMicro / (float)rampTimeMicro;
      currentDelay = (float)secondStartDelay - (delayDifference * rampPercent);
      rampElapsedMicro += currentDelay * 2; //We have to double the currentDelay because each step of the motor requires two pulses of the same delay
    }
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
      return firstSegmentRpm * adjust;
      break;
    case SECOND_SPD:
      return secondSegmentRpm * adjust;
      break;
    case FIRST_DEGREES:
      return (float)firstSegmentDegrees * adjust;
      break;
    case SECOND_DEGREES:
      return (float)secondSegmentDegrees * adjust;
      break;
    case RAMP_TIME:
      return rampTime * adjust;
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
    case RAMP_TIME:
      rampTime *= adjust;
      EEPROM_writeAnything(rampTimeStore, rampTime);
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
