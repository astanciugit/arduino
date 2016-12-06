#include "ToggleButtonLib.h"

ToggleButtonManager::ToggleButtonManager()
{
}

void ToggleButtonManager::init(int capacity)
{
}

void ToggleButtonManager::registerButton(int id, int pin)
{
/*
	ToggleButton btn = new ToggleButton(id, pin);
	int cnt = sizeof(_buttons)/sizeof(_buttons[0]);

	if (cnt >= _capaciry)
	{
		Serial.println("[ToggleButtonManager::checkStatuses] : out of bounds buttons array...");
		return;
	}
*/
//	Serial.print("[ToggleButtonManager::registerButton] : (1) count="); Serial.println(cnt);
	_buttons[id] = new ToggleButton(id, pin);
	pinMode(pin, INPUT);
//	Serial.print("[ToggleButtonManager::registerButton] : (2) count="); Serial.println(cnt);

}

//void ToggleButtonManager::registerButton(int id, int pin, void (*f)(int, bool))
//{
//}

void ToggleButtonManager::checkStatuses()
{
	if (_buttons[0] == NULL)
	{
		Serial.println("[ToggleButtonManager::checkStatuses] : not found registered buttons...");
		return;
	}

	int cnt = sizeof(_buttons)/sizeof(_buttons[0]);
	for (int i=0; i<cnt; i++)
	{

		ToggleButton *btn = _buttons[i];
		if (btn == NULL)
		{
			continue;
		}

		int reading = digitalRead(btn->_pin);
  		if (reading != btn->_lastButtonState) {
			lastDebounceTime = millis();
			btn->_canProcessButtonState = reading;
		}

	  	if (btn->_canProcessButtonState && (millis() - lastDebounceTime) > debounceDelay) {
			if (reading) {
				btn->_canProcessButtonState = false;
				btn->_buttonState = !btn->_buttonState;
//				buttonState1 ? setRelay(1, ON) : setRelay(1, OFF);
			}

		}

		btn->_lastButtonState = reading;
	}
}

bool ToggleButtonManager::getButtonStatus(int id)
{
	if (id >= _capaciry || _buttons[id] == NULL)
		return false;

	return _buttons[id]->_buttonState;
}

