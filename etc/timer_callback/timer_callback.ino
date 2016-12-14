//
// ESP8266 Timer Example
// SwitchDoc Labs  October 2015
//

// ===========   TIMER DEFINITION
extern "C" {
  #include "user_interface.h"
}
os_timer_t _myTimer;
bool _tickOccured;
bool _ledStatus = false;
unsigned int _timerDelay = 50;
unsigned int _ledDelay = 500;
// ===========   END OF TIMER DEFINITION

// ===========   TIMER COUNTERS
unsigned int _ms = 0;   // timer last checked value, milliseconds
unsigned int _ledDelayCounter = 0;  // LED timer last value, milliseconds
// ===========   END OFTIMER COUNTERS


// start of timerCallback
// NOTE: do not use Serial.println() in callback function
void timerCallback(void *pArg) {
  os_intr_lock();
    _tickOccured = true;
    checkTimer();
  os_intr_unlock();
} // End of timerCallback

void checkTimer() {
  // Milliseconds will overflow (go back to zero), after approximately 50 days.
  // https://www.arduino.cc/en/Reference/Millis
  unsigned int ms = millis();
  if (ms - _ms < 0 || _ms == 0) {
    _ms = ms;
    _ledDelayCounter = _ms;
  } else {
    _ms = ms;
  }
}


void user_init(void) {
/*
     os_timer_setfn - Define a function to be called when the timer fires
    
    void os_timer_setfn(
         os_timer_t *pTimer,
         os_timer_func_t *pFunction,
         void *pArg)
    
    Define the callback function that will be called when the timer reaches zero.
      The pTimer parameters is a pointer to the timer control structure.
      The pFunction parameters is a pointer to the callback function.
      The pArg parameter is a value that will be passed into the called back function. 
    
    The callback function should have the signature: void (*functionName)(void *pArg)
    The pArg parameter is the value registered with the callback function.
*/
     os_timer_setfn(&_myTimer, timerCallback, NULL);

/*
    os_timer_arm -  Enable a millisecond granularity timer.

    void os_timer_arm(
         os_timer_t *pTimer,
         uint32_t milliseconds,
         bool repeat)
    
    Arm a timer such that is starts ticking and fires when the clock reaches zero.
      The pTimer parameter is a pointed to a timer control structure.
      The milliseconds parameter is the duration of the timer measured in milliseconds. 
      The repeat parameter is whether or not the timer will restart once it has reached zero.
*/

    os_timer_arm(&_myTimer, _timerDelay, true);
} // End of user_init

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  Serial.println("");
  Serial.println("--------------------------");
  Serial.println("ESP8266 Timer Test");
  Serial.println("--------------------------");

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;
  _tickOccured = false;
  user_init();
}

void loop() {
  if (_tickOccured == true)
  {
    Serial.println("Tick Occurred");
    _tickOccured = false;
    if ((_ms - _ledDelayCounter) >= _ledDelay) {
      _ledStatus = !_ledStatus;
      _ledDelayCounter = _ms;
      digitalWrite(LED_BUILTIN, _ledStatus ? HIGH : LOW);
      Serial.println("===========   LED blink!!!");
    }
  }

  yield();  // or delay(0);
}
