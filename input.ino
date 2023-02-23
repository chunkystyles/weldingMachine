void handleInput(){
  switch (machineState) {
    case STOP:
      if (isStartStopButtonPressed()){
        machineState = RUN;
      } else if (isToggleButtonPressed()){
        handleToggle();
        delay(toggleDelay);
      } else if (isAdjustButtonPressed()){
        setupAdjust();
      }
      break;
    case RUN:
      //Nothing to do
      break;
    case ADJUST:
      if (isAdjustButtonPressed()){
        handleAdjust();
      } else {
        finalizeAdjust();
      }
      break;
    default:
      //Nothing to do
      break;
  }
}

void setupAdjust(){
  machineState = ADJUST;
  initialPotValue = analogRead(adjustPotPin);
}

void handleAdjust(){
  previousPotValue = analogRead(adjustPotPin);
  float adjustedValue = getAdjustedValue(calculateAdjustValue());
  if (toggle == FIRST_SPD || toggle == SECOND_SPD || toggle == RAMP_TIME){
    displayHackyDecimal(adjustedValue);
  } else {
    display.showNumberDec(roundUp(adjustedValue));
  }
  delay(adjustDelay);
}

void finalizeAdjust(){
  setAdjustedValue(calculateAdjustValue());
  machineState = STOP;
}

void handleToggle(){
  if (toggleStart > 0){
    switch (toggle) {
      case FIRST_SPD:
        toggle = SECOND_SPD;
        break;
      case SECOND_SPD:
        toggle = FIRST_DEGREES;
        break;
      case FIRST_DEGREES:
        toggle = SECOND_DEGREES;
        break;
      case SECOND_DEGREES:
        toggle = RAMP_TIME;
        break;
      case RAMP_TIME:
        toggle = FIRST_SPD;
        break;
      default:
        break;
    }
  }
  startToggle();
  displayToggle();
}

void startToggle(){
  toggleStart = millis();
  displayToggle();
}

void advanceToggleTimeout(){
  if (toggleStart > 0){
    long difference = millis() - toggleStart;
    if (difference >= toggleTimeout){
      toggleStart = 0;
      displayMachineState();
    }
  }
}

bool isStartStopButtonPressed(){
  return digitalRead(startStopButtonPin) == LOW;
}

bool isAdjustButtonPressed(){
  return digitalRead(adjustButtonPin) == LOW;
}

bool isToggleButtonPressed(){
  return digitalRead(toggleButtonPin) == LOW;
}

float calculateAdjustValue(){
  return 1.0 + ((float) (initialPotValue - previousPotValue)) * adjustIncrement;
}
