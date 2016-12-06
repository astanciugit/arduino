#ifndef ToggleButton_h
#define ToggleButton_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

class ToggleButton
{
	public:
		ToggleButton();
		ToggleButton(int id, int pin);
		ToggleButton(int id, int pin, void(*f)(int, bool));
		int _id = 0;
		int _pin = 0;
		bool _buttonState = false;
		bool _lastButtonState = false;
		bool _canProcessButtonState = false;
		void(*_func)(int, bool);
};


#endif