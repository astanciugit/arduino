#ifndef ToggleButtonLib_h
#define ToggleButtonLib_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 


class ToggleButtonManager
{
	public:
		ToggleButtonManager();
		void init(int capacity);
		void registerButton(int id, int pin);
		void checkStatuses();
		bool getButtonStatus(int id);
};


#endif