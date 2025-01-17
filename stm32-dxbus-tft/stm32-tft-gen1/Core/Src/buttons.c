
#include "main.h"
#include "buttons.h"
#include "ucm.h"
#include "canbus.h"
#include "hardwareinterface.h"
#include "powerManager.h"

uint8_t buttonField, buttonFieldOld;
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

void buttons_handleFlasherButtons(void) {
	if (((buttonFieldOld & BUTTON_MASK_RIGHT)==0) && ((buttonField & BUTTON_MASK_RIGHT)!=0)) {
		/* right button was just pushed */
		if (flasherMode == 1) {
			flasherMode = 0; /* if right side flashing was already running, then turn it off. */
		} else {
			flasherMode = 1; /* turn on the right side flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
	if (((buttonFieldOld & BUTTON_MASK_LEFT)==0) && ((buttonField & BUTTON_MASK_LEFT)!=0)) {
		/* left button was just pushed */
		if (flasherMode == 2) {
			flasherMode = 0; /* if left side flashing was already running, then turn it off. */
		} else {
			flasherMode = 2; /* turn on the left side flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
}

void buttons_mainfunction(void) { /* runs in 5ms cycle */
	buttonField = 0;
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)==0) buttonField |= 1;
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==0) buttonField |= 2;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)==0) buttonField |= 4;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)==0) buttonField |= 8;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)==0) buttonField |= 16;

	if (((buttonFieldOld & BUTTON_MASK_LIGHT)==0) && ((buttonField & BUTTON_MASK_LIGHT)!=0)) {
		if (blLightOn) blLightOn = 0; else blLightOn=1;
	}
	buttons_handleFlasherButtons();

	//blLightOn = (buttonField & BUTTON_MASK_LIGHT)!=0;
	buttons_handlePowerOffButton();

	buttonFieldOld = buttonField;
}
