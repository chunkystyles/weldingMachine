//All segments ----- SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define letter_S     SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
#define letter_t     SEG_D | SEG_E | SEG_F | SEG_G
#define letter_r     SEG_E | SEG_G
#define letter_o     SEG_C | SEG_D | SEG_E | SEG_G
#define letter_P     SEG_A | SEG_B | SEG_E | SEG_F | SEG_G
#define letter_u     SEG_C | SEG_D | SEG_E
#define letter_n     SEG_C | SEG_E | SEG_G
#define letter_A     SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
#define letter_d     SEG_B | SEG_C | SEG_D | SEG_E | SEG_G
#define letter_J     SEG_B | SEG_C | SEG_D | SEG_E
#define letter_E     SEG_A | SEG_D | SEG_E | SEG_F | SEG_G
#define letter_g     SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
#define letter_1     SEG_E | SEG_F
#define letter_2     SEG_A | SEG_B | SEG_D | SEG_E | SEG_G
#define letter_deg   SEG_A | SEG_B | SEG_F | SEG_G
#define timer_1      SEG_A
#define timer_2      SEG_A | SEG_B
#define timer_3      SEG_A | SEG_B | SEG_C
#define timer_4      SEG_A | SEG_B | SEG_C | SEG_D
#define timer_5      SEG_A | SEG_B | SEG_C | SEG_D | SEG_E
#define timer_6      SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
#define letter_blank 0b00000000
#define show_colon   0b01000000
#define timer_segments 7

const uint8_t display_stop[]     = { letter_S, letter_t,   letter_o, letter_P     };
const uint8_t display_run[]      = { letter_r, letter_u,   letter_n, letter_blank };
const uint8_t display_1spd[]     = { letter_1, letter_S,   letter_P, letter_d     };
const uint8_t display_2spd[]     = { letter_2, letter_S,   letter_P, letter_d     };
const uint8_t display_1degrees[] = { letter_1, letter_deg, letter_S, letter_blank };
const uint8_t display_2degrees[] = { letter_2, letter_deg, letter_S, letter_blank };
const uint8_t display_timer[]    = { letter_blank, timer_1, timer_2, timer_3, timer_4, timer_5, timer_6 };
const float timerFraction = 1.0 / ((float)timer_segments + 1.0);
int currentTimerSegment = -1;

void displayMachineState(){
  switch (machineState) {
    case STOP:
      display.setSegments(display_stop);
      break;
    case RUN:
      break;
    case ADJUST:
      //No real need to display anything here
      // other stuff will display instead
      break;
    default:
      break;
  }
}

void displayToggle(){
  switch (toggle) {
    case FIRST_SPD:
      display.setSegments(display_1spd);
      break;
    case SECOND_SPD:
      display.setSegments(display_2spd);
      break;
    case FIRST_DEGREES:
      display.setSegments(display_1degrees);
      break;
    case SECOND_DEGREES:
      display.setSegments(display_2degrees);
      break;
    default:
      break;
  }
}

void displayRun(){
    display.setSegments(display_run);
}

void displayHackyDecimal(float decimal){
  int left = decimal;
  int right = roundUp((decimal - (float)left) * (float)100);
  int whole = (left * 100) + right;
  display.showNumberDecEx(whole, (0b01000000), true);
}
