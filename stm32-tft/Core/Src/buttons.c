
#include "main.h"
#include "buttons.h"
#include "ucm.h"
#include "hardwareinterface.h"

uint8_t buttonField, buttonFieldOld;
uint8_t cntDebouncePowerOff;
#define DEBOUNCE_CYCLES_FOR_POWER_OFF 50 /* *5ms */


void buttons_handlePowerOffButton(void) {
	if (buttonField & BUTTON_MASK_POWER) {
		if (cntDebouncePowerOff<DEBOUNCE_CYCLES_FOR_POWER_OFF) {
			cntDebouncePowerOff++; /* debounce counter running */
		} else {
			setKeepPowerOn(0); /* debounce is finished -> turn off */
		}
	} else {
		cntDebouncePowerOff=0; /* reset the debounce counter */
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
	if (buttonField & BUTTON_MASK_RIGHT) flasherMode = 1;
	if (buttonField & BUTTON_MASK_LEFT) flasherMode = 2;

	//blLightOn = (buttonField & BUTTON_MASK_LIGHT)!=0;
	buttons_handlePowerOffButton();

	buttonFieldOld = buttonField;
}
