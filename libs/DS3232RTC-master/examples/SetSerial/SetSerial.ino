/*----------------------------------------------------------------------*
 * Display the date and time from a DS3231 or DS3232 RTC every second.  *
 * Display the temperature once per minute. (The DS3231 does a          *
 * temperature conversion once every 64 seconds. This is also the       *
 * default for the DS3232.)                                             *
 *                                                                      *
 * Set the date and time by entering the following on the Arduino       *
 * serial monitor:                                                      *
 *    year,month,day,hour,minute,second,                                *
 *                                                                      *
 * Where                                                                *
 *    year can be two or four digits,                                   *
 *    month is 1-12,                                                    *
 *    day is 1-31,                                                      *
 *    hour is 0-23, and                                                 *
 *    minute and second are 0-59.                                       *
 *                                                                      *
 * Entering the final comma delimiter (after &quot;second&quot;) will avoid a     *
 * one-second timeout and will allow the RTC to be set more accurately. *
 *                                                                      *
 * No validity checking is done, invalid values or incomplete syntax    *
 * in the input will result in an incorrect RTC setting.                *
 *                                                                      *
 * Jack Christensen 08Aug2013                                           *
 *                                                                      *
 * Tested with Arduino 1.0.5, Arduino Uno, DS3231/Chronodot, DS3232.    *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/ 
 
#include &lt;DS3232RTC.h&gt;        //http://github.com/JChristensen/DS3232RTC
#include &lt;Streaming.h&gt;        //http://arduiniana.org/libraries/streaming/
#include &lt;Time.h&gt;             //http://playground.arduino.cc/Code/Time
#include &lt;Wire.h&gt;             //http://arduino.cc/en/Reference/Wire

void setup(void)
{
    Serial.begin(115200);
    
    //setSyncProvider() causes the Time library to synchronize with the
    //external RTC by calling RTC.get() every five minutes by default.
    setSyncProvider(RTC.get);
    Serial &lt;&lt; F(&quot;RTC Sync&quot;);
    if (timeStatus() != timeSet) Serial &lt;&lt; F(&quot; FAIL!&quot;);
    Serial &lt;&lt; endl;
}

void loop(void)
{
    static time_t tLast;
    time_t t;
    tmElements_t tm;

    //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
    if (Serial.available() &gt;= 12) {
        //note that the tmElements_t Year member is an offset from 1970,
        //but the RTC wants the last two digits of the calendar year.
        //use the convenience macros from Time.h to do the conversions.
        int y = Serial.parseInt();
        if (y &gt;= 100 &amp;&amp; y &lt; 1000)
            Serial &lt;&lt; F(&quot;Error: Year must be two digits or four digits!&quot;) &lt;&lt; endl;
        else {
            if (y &gt;= 1000)
                tm.Year = CalendarYrToTm(y);
            else    //(y &lt; 100)
                tm.Year = y2kYearToTm(y);
            tm.Month = Serial.parseInt();
            tm.Day = Serial.parseInt();
            tm.Hour = Serial.parseInt();
            tm.Minute = Serial.parseInt();
            tm.Second = Serial.parseInt();
            t = makeTime(tm);
            RTC.set(t);        //use the time_t value to ensure correct weekday is set
            setTime(t);        
            Serial &lt;&lt; F(&quot;RTC set to: &quot;);
            printDateTime(t);
            Serial &lt;&lt; endl;
            //dump any extraneous input
            while (Serial.available() &gt; 0) Serial.read();
        }
    }
    
    t = now();
    if (t != tLast) {
        tLast = t;
        printDateTime(t);
        if (second(t) == 0) {
            float c = RTC.temperature() / 4.;
            float f = c * 9. / 5. + 32.;
            Serial &lt;&lt; F(&quot;  &quot;) &lt;&lt; c &lt;&lt; F(&quot; C  &quot;) &lt;&lt; f &lt;&lt; F(&quot; F&quot;);
        }
        Serial &lt;&lt; endl;
    }
}

//print date and time to Serial
void printDateTime(time_t t)
{
    printDate(t);
    Serial &lt;&lt; ' ';
    printTime(t);
}

//print time to Serial
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//print date to Serial
void printDate(time_t t)
{
    printI00(day(t), 0);
    Serial &lt;&lt; monthShortStr(month(t)) &lt;&lt; _DEC(year(t));
}

//Print an integer in &quot;00&quot; format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val &lt; 10) Serial &lt;&lt; '0';
    Serial &lt;&lt; _DEC(val);
    if (delim &gt; 0) Serial &lt;&lt; delim;
    return;
}