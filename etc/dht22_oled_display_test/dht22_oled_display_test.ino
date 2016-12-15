#include "DHT.h"
#include <TimeLib.h>
#include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Brzo.h"
#include "OLEDDisplayUi.h"  // Include the UI lib
#include "images.h" // Include custom images

// =========== OLED DISPLAY
// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D4 -> SCL
#define SDA_PIN D3
#define SCL_PIN D4
SSD1306Brzo _display(0x3c, SDA_PIN, SCL_PIN);
OLEDDisplayUi ui ( &_display );
int screenW = 128;
int screenH = 64;
int clockCenterX = screenW/2;
int clockCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int clockRadius = 23;
// FUNCTION PROTOTYPES
void relayOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void dhtFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);

// This array keeps function pointers to all frames are the single views that slide in
//FrameCallback frames[] = { analogClockFrame, digitalClockFrame, dhtFrame };
FrameCallback frames[] = { digitalClockFrame, dhtFrame };
int frameCount = 2;
// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { relayOverlay };
int overlaysCount = 1;
// =========== END OFF OLED DISPLAY

// ===========   TIMER DEFINITION
extern "C" {
  #include "user_interface.h"
}
os_timer_t _myTimer;
bool _tickOccured;
bool _ledStatus = false;
unsigned int _timerDelay = 50;
unsigned int _ledDelay = 500;
// ===========   END OF TIMER 

// ===========   TIMER COUNTERS
unsigned int _ms = 0;   // timer last checked value, milliseconds
unsigned int _ledDelayCounter = 0;  // LED timer last value, milliseconds
unsigned int _dhtDelayCounter = 0;  // DHT module timer last value, milliseconds
// ===========   END OFTIMER COUNTERS

// ===========   DHT22 DEFINITION
#define DHTPIN 3          // DHT PIN (GPIO3 D9)
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
unsigned int _dhtDelay = 5000;
float _hum;
float _temp;
float _hic;
// ===========   END OF DHT22 DEFINITION


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output for LED blink;
  _tickOccured = false;
  dht.begin();
  os_timer_setfn(&_myTimer, timerCallback, NULL);
  os_timer_arm(&_myTimer, _timerDelay, true);
  checkTimer();

  // The ESP is capable of rendering 60fps in 80Mhz mode but that won't give you much time 
  // for anything else run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(30);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  _display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  setTime(epoch);

}

void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // You can do some work here.
    // Don't do stuff if you are below your time budget.
    //delay(remainingTimeBudget);

    if (_tickOccured == true) {
      _tickOccured = false;
  
      if ((_ms - _ledDelayCounter) >= _ledDelay) {
        _ledStatus = !_ledStatus;
        _ledDelayCounter = _ms;
        digitalWrite(LED_BUILTIN, _ledStatus ? HIGH : LOW);
      }
  
      if ((_ms - _dhtDelayCounter) >= _dhtDelay) {
        _dhtDelayCounter = _ms;
        check_temperature();
        debugPrintTemperature();
      }
    }
  }

  yield();  // or delay(0);
}

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
    _dhtDelayCounter = _ms;
  } else {
    _ms = ms;
  }
}

void check_temperature() {
  _hum = dht.readHumidity();
  _temp = dht.readTemperature();
  if (!isnan(_hum) && !isnan(_temp)) {
    _hic = dht.computeHeatIndex(_temp, _hum, false);
  } else {
    Serial.println("[check_temperature] ERROR!!! WRONG TEMPERATURE AND HUMEDITY CHECKING!");
  }
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





// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    return '0'+String(digits);
  } else {
    return String(digits);
  }
}

void relayOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 , 0, "[1]");
  display->drawString(20 , 0, "[2]");
  display->drawString(96 , 0, "[3]");
  display->drawString(116 , 0, "[4]");
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  ui.disableIndicator();

  // Draw the clock face
//  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0Â° and stop at 360Â°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y, timenow );
}

void dhtFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String val1 = "T: "+String(_temp)+" C";
  String val2 = "H: "+String(_hum)+" %";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y - 24, val1);
  display->drawString(clockCenterX + x , clockCenterY + y, val2);
}

