#ifndef log_h
#define log_h

#include <Arduino.h>
#include <TimeAlarms.h>

void debugPrintDate(int yy, int mm, int dd) {
#ifdef DEBUG
  Serial.print(yy); Serial.print("-");
  Serial.print(mm); Serial.print("-");
  Serial.print(dd); Serial.print(" ");
#endif
}
void debugPrintDate() {
#ifdef DEBUG
  debugPrintDate(year(), month(), day());
#endif
}
void debugPrintDate(time_t tm) {
#ifdef DEBUG
  tmElements_t te;
  breakTime(tm, te);
  debugPrintDate(te.Year, te.Month, te.Day);
#endif
}


void debugPrintTime(int h, int m, int s) {
#ifdef DEBUG
  Serial.print(h); Serial.print(":");
  Serial.print(m); Serial.print(":");
  Serial.print(s); Serial.print(" ");
#endif
}
void debugPrintTime() {
#ifdef DEBUG
  debugPrintTime(hour(), minute(), second());
#endif
}
void debugPrintTime(time_t tm) {
#ifdef DEBUG
  tmElements_t te;
  breakTime(tm, te);
  debugPrintTime(te.Hour, te.Minute, te.Second);
#endif
}


void debugPrintDateTime(int yy, int mm, int dd, int h, int m, int s) {
#ifdef DEBUG
  debugPrintDate(yy, mm, dd);
  debugPrintTime(h, m, s);
#endif
}
void debugPrintDateTime(time_t tm) {
#ifdef DEBUG
  tmElements_t te;
  breakTime(tm, te);
  debugPrintDateTime(te.Year, te.Month, te.Day, te.Hour, te.Minute, te.Second);
#endif
}
void debugPrintNow() {
#ifdef DEBUG
  time_t tm = now();
  debugPrintDateTime(tm);
#endif
}

void debugPrint(char *ch) {
#ifdef DEBUG
  Serial.print(ch);
#endif
}
void debugPrint(int v) {
#ifdef DEBUG
  Serial.print(v);
#endif
}
void debugPrintln() {
#ifdef DEBUG
  Serial.println();
#endif
}
void debugPrintln(char *ch) {
#ifdef DEBUG
  Serial.println(ch);
#endif
}
void debugPrintln(int v) {
#ifdef DEBUG
  Serial.println(v);
#endif
}

#endif

