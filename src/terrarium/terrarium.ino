#include "DHT.h"
#include <Time.h>
#include <TimeLib.h>
//#include <TimeAlarms.h>

#define DEBUG
#define CONST_VER "0.1.1"

/* DHT DEFS & VARIABLES */
#define DHTPIN 2        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float _hum = NAN;
float _temp = NAN;
float _hic = NAN;
//=======   /* END OF: DHT DEFS & VARIABLES */

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
int _timerCounter = 0;
bool _tickOccured = false;
bool _ledStatus = false;
//=======   /* END OF: TIMER DEFS & VARIABLES */

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  setTime(6, 59, 0, 1, 1, 0); // (hh,mm,ss,MM,dd,yy) set time to Saturday 06:59:00am Jan 1 2000
  dht.begin();

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;
  pinMode(RELAY_PIN1, OUTPUT); digitalWrite(RELAY_PIN1, OFF);
  pinMode(RELAY_PIN2, OUTPUT); digitalWrite(RELAY_PIN2, OFF);
  pinMode(RELAY_PIN3, OUTPUT); digitalWrite(RELAY_PIN3, OFF);
  pinMode(RELAY_PIN4, OUTPUT); digitalWrite(RELAY_PIN4, OFF);

  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, _timerPeriod, true);

  delay(1000);

#ifdef DEBUG
  testRelay();
#endif
}

void loop() {
  check_temperature();

#ifdef DEBUG
  test();
#endif

  delay(500);
}

// start of timerCallback
// NOTE: do not use Serial.println() in callback function
void timerCallback(void *pArg) {
  os_intr_lock();
  if (_timerCounter == 10) {
    _timerCounter = 0;
    _tickOccured = true;
  }
  ++_timerCounter;
  _ledStatus = !_ledStatus;
  digitalWrite(LED_BUILTIN, _ledStatus);
  os_intr_unlock();
} // End of [timerCallback]


void check_temperature() {
  _hum = dht.readHumidity();
  _temp = dht.readTemperature();
  if (!isnan(_hum) && !isnan(_temp)) {
    _hic = dht.computeHeatIndex(_temp, _hum, false);
  } else {
    _hum = NAN;
    _temp = NAN;
    _hic = NAN;
#ifdef DEBUG
    Serial.println("[check_temperature] ERROR!!! WRONG TEMPERATURE AND HUMEDITY CHECKING!");
#endif
  }
}

void setRelay(int num, uint8_t v) {
  Serial.print("[setRelay] Relay #"); Serial.print(num); Serial.print(":");

  switch (num) {
    case 1:
      r1_status = v;
      digitalWrite(RELAY_PIN1, v);
      Serial.print(v); Serial.println();
      break;
    case 2:
      r2_status = v;
      digitalWrite(RELAY_PIN2, v);
      Serial.print(v); Serial.println();
      break;
    case 3:
      r3_status = v;
      digitalWrite(RELAY_PIN3, v);
      Serial.print(v); Serial.println();
      break;
    case 4:
      r4_status = v;
      digitalWrite(RELAY_PIN4, v);
      Serial.print(v); Serial.println();
      break;
    default:
      // TODO : update statistics !!!!!!!!!!!!!!!!!!!!!!!!!!
      Serial.print("!!! ERROR [setRelay]: was not found relay #"); Serial.println(num);
      break;
  }
} // END of [setRelay]

void test() {
#ifdef DEBUG
  debugPrintNow();
  debugPrintTemperature(); Serial.println();
#endif
}

void testRelay() {
  setRelay(1, ON); delay(500);
  setRelay(1, OFF); setRelay(2, ON); delay(500);
  setRelay(2, OFF); setRelay(3, ON); delay(500);
  setRelay(3, OFF); setRelay(4, ON); delay(500);
  setRelay(4, OFF); delay(500);
}

void debugPrintTemperature() {
  if (!isnan(_hum) && !isnan(_temp)) {
    Serial.print("Humidity: "); Serial.print(_hum); Serial.print("% \t");
    Serial.print("Temperature: "); Serial.print(_temp); Serial.print("C \t");
    Serial.print("Heat index: "); Serial.print(_hic); Serial.print(" *C ");
  } else {
    Serial.print("Failed to read from DHT sensor!");
    return;
  }
}



void debugPrintNow() {
#ifdef DEBUG
  Serial.print("[");
  Serial.print(year()); Serial.print("-");
  Serial.print(month()); Serial.print("-");
  Serial.print(day()); Serial.print(" ");
  Serial.print(hour()); Serial.print(":");
  Serial.print(minute()); Serial.print(":");
  Serial.print(second()); Serial.print(" ");
  Serial.print("] ");
#endif
}

