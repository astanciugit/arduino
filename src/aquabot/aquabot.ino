#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>
#include <Time.h>
#include <DS3232RTC.h>

#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

const uint8_t PIN_RELAY = 5;      // Relay PIN
const uint8_t PIN_INDICATOR = 13;   // Activity indicator PIN

const byte sensors = 3;
byte sensorPin[] = {8, 9, 10};
const uint8_t LED_PIN_MIN_LEVEL = 11; // LED for MIN water level - 1 if water level is >= MIN
const uint8_t LED_PIN_MAX_LEVEL = 12; // LED for MAX water level - 1 if water level is >= MAX
int _waterLevel = 0; 

time_t _tm;
bool _isRelayOn = false;
unsigned long _lastRTCTime = millis();    // 0.5 sec timer's last value

const int16_t pos_line_1_Y      = 0;
const int16_t pos_line_2_Y      = 26;
const int16_t pos_line_3_Y      = 42;
const int16_t pos_line_4_Y      = 54;
const int16_t pos_size1_offset_Y  = 6;

bool _isEvenPeriod = false;
char* DaysOfWeekShort[] = { "Err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define __printd(a) { if (a < 10) oled.print(0); oled.print(a); }

void setup()   {                
  Serial.begin(115200);
  Serial.println("Serial init done");
  
  pinMode(PIN_RELAY, OUTPUT);
  Serial.println("Relay OUTPUT PIN init OUTPUT is done");
  pinMode(PIN_INDICATOR, OUTPUT);
  Serial.println("Relay indicator PIN init OUTPUT is done");
  
  for(int i = 0; i < sensors; i++) {
    pinMode(sensorPin[i], INPUT); 
  }
  
  pinMode(LED_PIN_MIN_LEVEL, OUTPUT);
  pinMode(LED_PIN_MAX_LEVEL, OUTPUT);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  Serial.println("Show splashscreen...");
  oled.display(); // show splashscreen
  delay(1000);
  oled.clearDisplay();   // clears the screen and buffer
  oled.setTextSize(2);
  oled.setTextColor(WHITE);

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus() != timeSet) 
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  
  //setDateTime(2016,5,24, 3, 19,50,45);  //yyyy,MM,dd,WD,hh,mm,ss
}

void setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekDay,
         uint8_t hour, uint8_t minute, uint8_t second) {
  tmElements_t tm;
  
  tm.Year = CalendarYrToTm(year);
  tm.Month = month;
  tm.Day = day;
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = second;
  tm.Wday = weekDay;
  RTC.write(tm);
}

// draw a bitmap icon and 'animate' movement
void loop() {
  if (!_isRelayOn && _waterLevel >= 3)
    setRelay(true);
  else if (_isRelayOn && _waterLevel <= 1)
    setRelay(false);
  
  if (millis() - _lastRTCTime > 500) {
    _lastRTCTime = millis();
    _isEvenPeriod = !_isEvenPeriod;
    digitalWrite(PIN_INDICATOR, _isEvenPeriod ? 1 : 0);
    _tm = RTC.get();
    processHydrometer();
    displayMainScreen(_tm);
  }
}

void processHydrometer() {
  int waterLevel = 4; 
  for(int i = 0; i < sensors; i++) {
    int val = digitalRead(sensorPin[i]);
    Serial.print(i); Serial.print("/["); Serial.print(sensorPin[i]); Serial.print("]="); Serial.print(val); Serial.print("; "); 
    if(val == LOW) {
      //digitalWrite(ledPin[i], HIGH);
      Serial.print("[LOW] ");
      waterLevel = sensors - i;
    } else {
      Serial.print("[HIGH] ");
      //digitalWrite(ledPin[i], LOW);       
    }    
  }
  
  _waterLevel = waterLevel;
  Serial.print("level="); Serial.print(_waterLevel); Serial.print("; ");
  Serial.println();
}

void displayMainScreen(time_t &tm) {
  oled.clearDisplay();
  displayStandardScreen(tm);
  oled.display();
}

/* Displaying non-editing mode */
void displayStandardScreen(time_t &tm) {
  // Time
  oled.setCursor(0,pos_line_1_Y);
  oled.setTextSize(3);
  __printd(hour(tm));
  oled.print(_isEvenPeriod ? ':' : ' ');
  __printd(minute(tm));
  oled.setCursor(94,pos_line_1_Y + pos_size1_offset_Y);
  oled.setTextSize(2);
  __printd(second(tm));

  // date & weekday 
  oled.setCursor(32,pos_line_2_Y);
  oled.setTextSize(1);
  oled.print(monthShortStr(month(tm)));
  oled.print(' ');
  oled.print(day(tm));
  oled.print(", ");
  oled.print(DaysOfWeekShort[weekday(tm)]);
  
  // hydro
  char s = ' ';
  oled.setCursor(0, pos_line_3_Y);
  oled.setTextSize(1);
  displayHydroInfo(_waterLevel);
}

void displayHydroInfo(int waterLevel) {
  oled.print("[");
  oled.print(waterLevel);
  oled.print("]   ");
  if (_waterLevel == 1)
    oled.print("HIGH");
  else if (_waterLevel == 2)
    oled.print("MEDIUM");
  else if (_waterLevel == 3)
    oled.print("LOW");
  else
    oled.print("---");
}

void setRelay(bool isRelayOn) {
  _isRelayOn = isRelayOn;
  digitalWrite(PIN_RELAY, _isRelayOn ? HIGH : LOW);
  Serial.print("isRelayOn: ");
  Serial.println(_isRelayOn);
}

