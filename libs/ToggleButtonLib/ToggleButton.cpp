#include "ToggleButton.h"

ToggleButton::ToggleButton()
{
	_id = -1;
	_pin = -1;
}

ToggleButton::ToggleButton(int id, int pin)
{
	_id = id;
	_pin = pin;
}

ToggleButton::ToggleButton(int id, int pin, void(*f)(int, bool))
{
	_id = id;
	_pin = pin;
	_func = f;
}
