#include <ToggleButtonLib.h>

#define DEBUG
#define CONST_VER "0.2.1"

/* RELAY DEFS & VARIABLES */
#define RELAY_PIN1 14  // Relay 1 PIN (GPIO14  D5)
#define RELAY_PIN2 12  // Relay 2 PIN (GPIO12 D6)
#define RELAY_PIN3 13  // Relay 3 PIN (GPIO13 D7)
#define RELAY_PIN4 15  // Relay 4 PIN (GPIO15 D8)
#define ON  LOW   // Relay ON signal
#define OFF HIGH  // Relay OFF signal
bool _R1_status = false;
bool _R2_status = false;
bool _R3_status = false;
bool _R4_status = false;
//=======   /* END OF: RELAY DEFS & VARIABLES */

/* TIMER DEFS & VARIABLES */
extern "C" {
  #include "user_interface.h"
}
os_timer_t _myTimer;
bool _tickOccured = false;
bool _ledStatus = false;
const int _timerDelay = 50;
const int _ledDelay = 500;
unsigned int _relayDelay = 50;
//=======   /* END OF: TIMER DEFS & VARIABLES */

// ===========   TIMER COUNTERS
unsigned int _ms = 0;   // timer last checked value, milliseconds
unsigned int _ledDelayCounter = 0;  // LED timer last value, milliseconds
unsigned int _relayDelayCounter = 0;  // RELAY module timer last value, milliseconds
// ===========   END OFTIMER COUNTERS

/* BUTTONS DEFS & VARIABLES */
#define BTN_PIN1 5  // pushbutton 1 PIN (GPIO5  D1) - UV & daytime lamps;
#define BTN_PIN2 4  // pushbutton 2 PIN (GPIO4  D2) - IR nightime lamp;
#define BTN1 1
#define BTN2 2
ToggleButtonManager _btnManager;
//=======   /* END OF: BUTTONS DEFS & VARIABLES */

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;
  pinMode(RELAY_PIN1, OUTPUT); digitalWrite(RELAY_PIN1, OFF);
  pinMode(RELAY_PIN2, OUTPUT); digitalWrite(RELAY_PIN2, OFF);
  pinMode(RELAY_PIN3, OUTPUT); digitalWrite(RELAY_PIN3, OFF);
  pinMode(RELAY_PIN4, OUTPUT); digitalWrite(RELAY_PIN4, OFF);

  Serial.println("PINs was setup successfull...");

  _btnManager.init(2);
  //_btnManager.registerButton(BTN1, BTN_PIN1);
  //_btnManager.registerButton(BTN2, BTN_PIN2);
  _btnManager.registerButton(BTN1, BTN_PIN1, handleButton);
  _btnManager.registerButton(BTN2, BTN_PIN2, handleButton);

  os_timer_setfn(&_myTimer, (os_timer_func_t *)timerCallback, NULL);
  os_timer_arm(&_myTimer, _timerDelay, true);
  Serial.println("Timer was setup successfull...");
}

void loop() {
  if (_tickOccured == true) {
    _tickOccured = false;

      if ((_ms - _ledDelayCounter) >= _ledDelay) {
        _ledStatus = !_ledStatus;
        _ledDelayCounter = _ms;
        digitalWrite(LED_BUILTIN, _ledStatus ? HIGH : LOW);
      }

      if ((_ms - _relayDelayCounter) >= _relayDelay) {
        _relayDelayCounter = _ms;
        _btnManager.checkStatuses();
        check_button();
      }
  }

  yield();  // or delay(0);
}

// start of timerCallback
// NOTE: do not use Serial.println() in callback function
void timerCallback(void *pArg) {
  os_intr_lock();
    _tickOccured = true;
    checkTimer();
  os_intr_unlock();
} // End of [timerCallback]

void checkTimer() {
  // Milliseconds will overflow (go back to zero), after approximately 50 days.
  // https://www.arduino.cc/en/Reference/Millis
  unsigned int ms = millis();
  if (ms - _ms < 0 || _ms == 0) {
    _ms = ms;
    _ledDelayCounter = _ms;
    _relayDelayCounter = _ms;
  } else {
    _ms = ms;
  }
} // End of [checkTimer]

void handleButton(int id, bool val) {
  uint8_t v = val ? ON : OFF;
  setRelay(id, v);
  Serial.print("[handleButton]: relay #"); Serial.print(id); Serial.print(" status was set to "); Serial.print(v); Serial.println();
} // End of [handleButton]

int check_button() {
  bool status1 = _btnManager.getButtonStatus(BTN1);
  //setRelay(1, status1 ? ON : OFF);
  bool status2 = _btnManager.getButtonStatus(BTN2);
  //setRelay(2, status2 ? ON : OFF);
  Serial.print("status1="); Serial.print(status1); Serial.print("; status2="); Serial.print(status2); Serial.println();
}

void setRelay(int num, uint8_t v) {
  Serial.print("[setRelay] Relay #"); Serial.print(num); Serial.print(":");

  switch (num) {
    case 1:
      _R1_status = v;
      digitalWrite(RELAY_PIN1, v);
      Serial.print(v);
      break;
    case 2:
      _R2_status = v;
      digitalWrite(RELAY_PIN2, v);
      Serial.print(v);
      break;
    case 3:
      _R3_status = v;
      digitalWrite(RELAY_PIN3, v);
      Serial.print(v);
      break;
    case 4:
      _R4_status = v;
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
