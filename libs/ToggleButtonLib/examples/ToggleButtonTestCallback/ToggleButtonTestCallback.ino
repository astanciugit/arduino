#include <ToggleButtonLib.h>

/* TIMER DEFS & VARIABLES */
extern "C" {
  #include "user_interface.h"
}
os_timer_t myTimer;
const int _timerDelay = 50;
const int _timerPeriod = 500;
int _timerCounter = 0;
bool _tickOccured = false;
bool _ledStatus = false;
//=======   /* END OF: TIMER DEFS & VARIABLES */

/* BUTTONS DEFS & VARIABLES */
#define BTN_PIN1 5  // pushbutton 1 (GPIO3 D1) - UV & daytime lamps;
#define BTN_PIN2 4  // pushbutton 2 (GPIO1 D2) - IR nightime lamp;
#define BTN1 1	// button ID
#define BTN2 2	// button ID
ToggleButtonManager _btnManager;
//=======   /* END OF: BUTTONS DEFS & VARIABLES */

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;

  _btnManager.init(2);
  _btnManager.registerButton(BTN1, BTN_PIN1, handleButton);
  _btnManager.registerButton(BTN2, BTN_PIN2, handleButton);

  os_timer_setfn(&myTimer, (os_timer_func_t *)timerCallback, NULL);
  os_timer_arm(&myTimer, _timerDelay, true);
  Serial.println("Timer was setup successfull...");
}

void loop() {
  // put your main code here, to run repeatedly:
}

// start of timerCallback
// NOTE: do not use Serial.println() in callback function
void timerCallback(void *pArg) {
  os_intr_lock();
    ++_timerCounter;
    if (_timerCounter * _timerDelay >= _timerPeriod) {
      _timerCounter = 0;
      _tickOccured = true;
      _ledStatus = !_ledStatus;
      _btnManager.checkStatuses();
      digitalWrite(LED_BUILTIN, _ledStatus);
    }
  os_intr_unlock();
} // End of [timerCallback]

void handleButton(int id, bool val) {
  Serial.print("[handleButton]: button #"); Serial.print(id); Serial.print(" status was set to "); Serial.print(val); Serial.println();
} // End of [handleButton]
