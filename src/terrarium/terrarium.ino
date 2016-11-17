#define DEBUG
#include "DHT.h"
#include <TimeAlarms.h>
#include "log.h"


#define DHTPIN 2        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);
float _hum;
float _temp;
float _hic;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  dht.begin();
}

void loop() {
  check_temperature();

#ifdef DEBUG
  test();
#endif

  delay(500);
}

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

void test() {
  debugPrintTemperature();
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
  
  Serial.println();
}

