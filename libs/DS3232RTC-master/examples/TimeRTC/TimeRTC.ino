/*
 * TimeRTC.pde
 * Example code illustrating Time library with Real Time Clock.
 * This example is identical to the example provided with the Time Library,
 * only the #include statement has been changed to include the DS3232RTC library.
 */

#include &lt;DS3232RTC.h&gt;    //http://github.com/JChristensen/DS3232RTC
#include &lt;Time.h&gt;         //http://www.arduino.cc/playground/Code/Time  
#include &lt;Wire.h&gt;         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)

void setup(void)
{
    Serial.begin(9600);
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet) 
        Serial.println(&quot;Unable to sync with the RTC&quot;);
    else
        Serial.println(&quot;RTC has set the system time&quot;);      
}

void loop(void)
{
    digitalClockDisplay();  
    delay(1000);
}

void digitalClockDisplay(void)
{
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(' ');
    Serial.print(day());
    Serial.print(' ');
    Serial.print(month());
    Serial.print(' ');
    Serial.print(year()); 
    Serial.println(); 
}

void printDigits(int digits)
{
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if(digits &lt; 10)
        Serial.print('0');
    Serial.print(digits);
}
