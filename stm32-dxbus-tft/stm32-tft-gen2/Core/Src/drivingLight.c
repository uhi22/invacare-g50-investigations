
#include "drivingLight.h"
#include "hardwareAbstraction.h"

uint8_t oldLightButton;
uint8_t lightMode;

void light_handleButton(uint8_t lightbutton) {
	if (lightbutton && !oldLightButton) {
		/*  button was just pushed */
		if (lightMode == 1) {
			lightMode = 0; /* if light was on, then turn it off. */
		} else {
			lightMode = 1; /* turn light on */
		}
	}
	oldLightButton = lightbutton;
}


void light_mainfunction20ms(void) {
	switch (lightMode) {
		case 1: /* on */
			setOut2(1);
			break;
		default: /* off */
			setOut2(0);
	}
}
