#ifndef ToggleButtonLib_h
#define ToggleButtonLib_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#include "ToggleButton.h"

class ToggleButtonManager
{
	public:
		ToggleButtonManager();
		void init(int capacity);
		void registerButton(int id, int pin);
//		void registerButton(int id, int pin, void (*f)(int, bool));
		void checkStatuses();
		bool getButtonStatus(int id);

		unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
		unsigned const long debounceDelay = 50;    // the debounce time; increase if the output flickers

	private:
		int const _capaciry = 4;
		ToggleButton *_buttons[4] = {};
};


#endif