#include "ToggleButton.h"

ToggleButton::ToggleButton()
{
}

ToggleButton::ToggleButton(int id, int pin)
{
	_id = id;
	_pin = pin;
	_buttonState = false;
}

ToggleButton::ToggleButton(int id, int pin, void(*f)(int, bool))
{
	_id = id;
	_pin = pin;
	_buttonState = false;
	_func = f;
}