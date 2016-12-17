#include "DHT.h"
#include <TimeLib.h>
#include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Brzo.h"
#include "OLEDDisplayUi.h"  // Include the UI lib
#include <ToggleButtonLib.h>
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

/* BUTTONS DEFS & VARIABLES */
// Toggle buttons logic: button pins should be pulled up (resistor 10kOhm on +5V);
// Button will be pressed on 0 (resistor 10kOhm on Gnd) and released on 1 (resistor 10kOhm on +5V);
// Pressing timout should be not less that 50 ms (variable   _buttonDelay);
#define BTN_PIN1 5  // pushbutton 1 PIN (GPIO5  D1) - UV & daytime lamps;
#define BTN_PIN2 4  // pushbutton 2 PIN (GPIO4  D2) - IR nightime lamp;
#define BTN1 1
#define BTN2 2
ToggleButtonManager _btnManager;
//=======   /* END OF: BUTTONS DEFS & VARIABLES */

// ===========   TIMER DEFINITION
extern "C" {
  #include "user_interface.h"
}
os_timer_t _myTimer;
bool _tickOccured;
bool _ledStatus = false;
unsigned int _timerDelay = 50;
unsigned int _ledDelay = 500;
unsigned int CONST_DEBOUNCE_DELAY = 50;
// ===========   END OF TIMER 

// ===========   TIMER COUNTERS
unsigned int _ms = 0;   // timer last checked value, milliseconds
unsigned int _ledDelayCounter = 0;  // LED timer last value, milliseconds
unsigned int _dhtDelayCounter = 0;  // DHT module timer last value, milliseconds
unsigned int _buttonDelayCounter = 0;  // RELAY module timer last value, milliseconds
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
  pinMode(RELAY_PIN1, OUTPUT); digitalWrite(RELAY_PIN1, OFF);
  pinMode(RELAY_PIN2, OUTPUT); digitalWrite(RELAY_PIN2, OFF);
  pinMode(RELAY_PIN3, OUTPUT); digitalWrite(RELAY_PIN3, OFF);
  pinMode(RELAY_PIN4, OUTPUT); digitalWrite(RELAY_PIN4, OFF);
  setRelay(1, OFF);
  setRelay(2, OFF);
  setRelay(3, OFF);
  setRelay(4, OFF);

  _btnManager.init(2, CONST_DEBOUNCE_DELAY);
  _btnManager.registerButton(BTN1, BTN_PIN1, handleButton);
  _btnManager.registerButton(BTN2, BTN_PIN2, handleButton);

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
      _btnManager.checkStatuses();
  
      if ((_ms - _ledDelayCounter) >= _ledDelay) {
        _ledStatus = !_ledStatus;
        _ledDelayCounter = _ms;
        digitalWrite(LED_BUILTIN, _ledStatus ? HIGH : LOW);
        check_button();
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

void setRelay(int num, uint8_t val) {
  Serial.print("[setRelay] Relay #"); Serial.print(num); Serial.print(":");

  uint8_t v = !val;
  switch (num) {
    case 1:
      _R1_status = v;
      digitalWrite(RELAY_PIN1, val);
      Serial.print(v);
      break;
    case 2:
      _R2_status = v;
      digitalWrite(RELAY_PIN2, val);
      Serial.print(v);
      break;
    case 3:
      _R3_status = v;
      digitalWrite(RELAY_PIN3, val);
      Serial.print(v);
      break;
    case 4:
      _R4_status = v;
      digitalWrite(RELAY_PIN4, val);
      Serial.print(v);
      break;
    default:
      // TODO : update statistics !!!!!!!!!!!!!!!!!!!!!!!!!!
      Serial.print("!!! ERROR [setRelay]: was not found relay #"); Serial.print(num);
      break;
  }

  Serial.println();
} // END of [setRelay]





// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    return '0'+String(digits);
  } else {
    return String(digits);
  }
}

const String CONST_DISABLED_RELAY_LABEL = "[--]";
void relayOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 , 0, _R1_status ? "[1]" : CONST_DISABLED_RELAY_LABEL);
  display->drawString(20 , 0, _R2_status ? "[2]" : CONST_DISABLED_RELAY_LABEL);
  display->drawString(96 , 0, _R3_status ? "[3]" : CONST_DISABLED_RELAY_LABEL);
  display->drawString(116 , 0, _R4_status ? "[4]" : CONST_DISABLED_RELAY_LABEL);
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


