#define DEBUG
#define CONST_VER "0.1.1"

/* RELAY DEFS & VARIABLES */
#define RELAY_PIN1 14  // Relay 1 PIN (D5)
#define RELAY_PIN2 12  // Relay 2 PIN (D6)
#define RELAY_PIN3 13  // Relay 3 PIN (D7)
#define RELAY_PIN4 15  // Relay 4 PIN (D8)
#define ON  LOW   // Relay ON signal
#define OFF HIGH  // Relay OFF signal
bool r1_status = false;
bool r2_status = false;
bool r3_status = false;
bool r4_status = false;
//=======   /* END OF: RELAY DEFS & VARIABLES */

/* TIMER DEFS & VARIABLES */
extern "C" {
  #include "user_interface.h"
}
os_timer_t myTimer;
const int _timerPeriod = 500;
bool _tickOccured = false;
bool _ledStatus = false;
//=======   /* END OF: TIMER DEFS & VARIABLES */

/* BUTTONS DEFS & VARIABLES */
#define BTN_PIN1 5  // pushbutton 1 (GPIO3 D1) - UV & daytime lamps;
#define BTN_PIN2 4  // pushbutton 2 (GPIO1 D2) - IR nightime lamp;
bool lastButtonState1 = false;  // the previous reading pushbutton 1
bool lastButtonState2 = false;  // the previous reading pushbutton 2
bool buttonState1 = false;      // the current reading pushbutton 1
bool buttonState2 = false;      // the current reading pushbutton 2
bool canProcessButtonState = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned const long debounceDelay = 50;    // the debounce time; increase if the output flickers
//=======   /* END OF: BUTTONS DEFS & VARIABLES */

int check_button() {
  int reading1 = digitalRead(BTN_PIN1);
  int reading2 = digitalRead(BTN_PIN2);
  Serial.print("[check_button]"); 
  Serial.print(" reading1="); Serial.print(reading1);
  Serial.print(" reading2="); Serial.print(reading2);

  if (reading1 != lastButtonState1 || reading2 != lastButtonState2) {
    Serial.print("LOOP_01...");
    lastDebounceTime = millis();
    canProcessButtonState = reading1 || reading2;
  }

  if (canProcessButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    Serial.print("LOOP_02...");
    if (reading1) {
      canProcessButtonState = false;
      Serial.print("LOOP_03...");
      buttonState1 = !buttonState1;
      buttonState1 ? setRelay(1, ON) : setRelay(1, OFF);
    }

    if (reading2) {
      canProcessButtonState = false;
      Serial.print("LOOP_04...");
      buttonState2 = !buttonState2;
      buttonState2 ? setRelay(2, ON) : setRelay(2, OFF);
    }
  }

  Serial.println();

  lastButtonState1 = reading1;
  lastButtonState2 = reading2;
}


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;
  pinMode(RELAY_PIN1, OUTPUT); digitalWrite(RELAY_PIN1, OFF);
  pinMode(RELAY_PIN2, OUTPUT); digitalWrite(RELAY_PIN2, OFF);
  pinMode(RELAY_PIN3, OUTPUT); digitalWrite(RELAY_PIN3, OFF);
  pinMode(RELAY_PIN4, OUTPUT); digitalWrite(RELAY_PIN4, OFF);

  pinMode(BTN_PIN1, INPUT); digitalWrite(BTN_PIN1, OFF);
  pinMode(BTN_PIN2, INPUT); digitalWrite(BTN_PIN2, OFF);

  Serial.println("PINs was setup successfull...");

/*
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, _timerPeriod, true);
  Serial.println("Timer was setup successfull...");
*/
}

void loop() {
  check_button();
  delay(100);
}

// start of timerCallback
// NOTE: do not use Serial.println() in callback function
void timerCallback(void *pArg) {
  os_intr_lock();
    _tickOccured = true;
    _ledStatus = !_ledStatus;
    digitalWrite(LED_BUILTIN, _ledStatus);
  os_intr_unlock();
} // End of [timerCallback]

void setRelay(int num, uint8_t v) {
  Serial.print("[setRelay] Relay #"); Serial.print(num); Serial.print(":");

  switch (num) {
    case 1:
      r1_status = v;
      digitalWrite(RELAY_PIN1, v);
      Serial.print(v);
      break;
    case 2:
      r2_status = v;
      digitalWrite(RELAY_PIN2, v);
      Serial.print(v);
      break;
    case 3:
      r3_status = v;
      digitalWrite(RELAY_PIN3, v);
      Serial.print(v);
      break;
    case 4:
      r4_status = v;
      digitalWrite(RELAY_PIN4, v);
      Serial.print(v);
      break;
    default:
      // TODO : update statistics !!!!!!!!!!!!!!!!!!!!!!!!!!
      Serial.print("!!! ERROR [setRelay]: was not found relay #"); Serial.print(num);
      break;
  }

  Serial.println();
} // END of [setRelay]


