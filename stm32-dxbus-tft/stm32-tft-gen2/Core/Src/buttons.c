
#include "main.h"
#include "buttons.h"
//#include "ucm.h"
//#include "canbus.h"
#include "hardwareAbstraction.h"
#include "powerManager.h"
#include "turnIndicator.h"
#include "drivingLight.h"

uint16_t buttonField, buttonFieldOld;
uint8_t cntDebouncePowerOff;
#define DEBOUNCE_CYCLES_FOR_POWER_OFF 50 /* *5ms */


void buttons_handlePowerOffButton(void) {
	if (buttonField & BUTTON_MASK_POWER) {
		if (cntDebouncePowerOff<DEBOUNCE_CYCLES_FOR_POWER_OFF) {
			cntDebouncePowerOff++; /* debounce counter running */
		} else {
			pwrM_startThePowerOffSequence();
		}
	} else {
		cntDebouncePowerOff=0; /* reset the debounce counter */
	}

}



void buttons_mainfunction(void) { /* runs in 5ms cycle */
	buttonField = 0;
	if (getButton1()) buttonField |= BUTTON_MASK_RIGHT;
	if (getButton2()) buttonField |= BUTTON_MASK_BUTTON2;
	if (getButton3()) buttonField |= BUTTON_MASK_MIDDLE;
	if (getButton4()) buttonField |= BUTTON_MASK_LEFT;
	if (getJoystickButton()) buttonField |= BUTTON_MASK_JOYSTICK;
	if (getPowerButton()) buttonField |= BUTTON_MASK_POWER;

	if (getSwitch1()) buttonField |= BUTTON_MASK_SWITCH1;
	if (getSwitch2()) buttonField |= BUTTON_MASK_SWITCH2;
	if (getSwitch3()) buttonField |= BUTTON_MASK_SWITCH3;
	if (getSwitch4()) buttonField |= BUTTON_MASK_SWITCH4;


	if (((buttonFieldOld & BUTTON_MASK_LIGHT)==0) && ((buttonField & BUTTON_MASK_LIGHT)!=0)) {
		//if (blLightOn) blLightOn = 0; else blLightOn=1;
	}
	turni_handleButtons(buttonField & BUTTON_MASK_LEFT, buttonField & BUTTON_MASK_RIGHT /*, buttonField & BUTTON_MASK_JOYSTICK */ );
	light_handleButton(buttonField & BUTTON_MASK_MIDDLE);
	//blLightOn = (buttonField & BUTTON_MASK_LIGHT)!=0;
	buttons_handlePowerOffButton();

	buttonFieldOld = buttonField;
}
