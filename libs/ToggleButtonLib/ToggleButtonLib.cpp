#include "ToggleButtonLib.h"

ToggleButtonManager::ToggleButtonManager()
{
}

void ToggleButtonManager::init(int capacity)
{
}

void ToggleButtonManager::registerButton(int id, int pin)
{
	_buttons[_counter] = new ToggleButton(id, pin);
	++_counter;
	pinMode(pin, INPUT);
}

void ToggleButtonManager::registerButton(int id, int pin, void (*f)(int, bool))
{
	_buttons[_counter] = new ToggleButton(id, pin, f);
	++_counter;
	pinMode(pin, INPUT);
}

void ToggleButtonManager::checkStatuses()
{
	if (_buttons[0] == NULL)
	{
		return;
	}

	for (int i=0; i<_capacity; i++)
	{

		ToggleButton *btn = _buttons[i];
		if (btn == NULL)
			continue;

		int reading = digitalRead(btn->_pin);
  		if (reading != btn->_lastButtonState)
		{
			lastDebounceTime = millis();
			btn->_canProcessButtonState = reading;
		}

	  	if (btn->_canProcessButtonState && (millis() - lastDebounceTime) > debounceDelay)
		{
			if (reading)
			{
				btn->_canProcessButtonState = false;
				btn->_buttonState = !btn->_buttonState;
				if (btn->_func != NULL)
					btn->_func(btn->_id, btn->_buttonState);
			}

		}

		btn->_lastButtonState = reading;
	}
}

bool ToggleButtonManager::getButtonStatus(int id)
{
	for (int i=0; i<_capacity; i++)
	{
		if (_buttons[i] != NULL && _buttons[i]->_id == id)
			return _buttons[i]->_buttonState;
	}


	return false;
}
