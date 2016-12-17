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
		void init(unsigned int capacity, unsigned int debounceDelay);
		void registerButton(int id, int pin);
		void registerButton(int id, int pin, void (*f)(int, bool));
		void checkStatuses();
		bool getButtonStatus(int id);

	private:
		unsigned int const _capacity = 4;
		unsigned int _debounceDelay = 50;    // the debounce time; increase if the output flickers
		unsigned long _lastDebounceTime = 0;  // the last time the output pin was toggled
		int _counter = 0;
		ToggleButton *_buttons[4] = {};
};

#endif
