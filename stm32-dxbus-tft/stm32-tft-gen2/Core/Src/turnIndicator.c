
#include "turnIndicator.h"
#include "hardwareAbstraction.h"

uint8_t oldRightButton;
uint8_t oldLeftButton;
uint8_t oldHazardButton;
uint8_t turnIndicatorMode;
uint8_t flasherDivider;

void turni_handleButtons(uint8_t leftbutton, uint8_t rightbutton, uint8_t hazardButton) {
	if (rightbutton && !oldRightButton) {
		/* right button was just pushed */
		if (turnIndicatorMode == 1) {
			turnIndicatorMode = 0; /* if right side flashing was already running, then turn it off. */
		} else {
			turnIndicatorMode = 1; /* turn on the right side flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
	if (leftbutton && !oldLeftButton) {
		/* left button was just pushed */
		if (turnIndicatorMode == 2) {
			turnIndicatorMode = 0; /* if left side flashing was already running, then turn it off. */
		} else {
			turnIndicatorMode = 2; /* turn on the left side flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
	if (hazardButton && !oldHazardButton) {
		if (turnIndicatorMode == 3) {
			turnIndicatorMode = 0; /* if hazard flashing was already running, then turn it off. */
		} else {
			turnIndicatorMode = 3; /* turn on the hazard flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
	oldLeftButton = leftbutton;
	oldRightButton = rightbutton;
	oldHazardButton = hazardButton;
}

#define CYCLE_TIME_20MS (760/20)
#define ON_TIME_20MS (380/20)
#define CYCLE_TIME_HAZARD_20MS (680/20)
#define ON_TIME_HAZARD_20MS (340/20)

void turni_mainfunction20ms(void) {
	flasherDivider++;
	if (flasherDivider>=CYCLE_TIME_20MS) flasherDivider=0;
	switch (turnIndicatorMode) {
		case 1: /* right turn */
			setLED_D1(flasherDivider<ON_TIME_20MS);
			setLED_D11(0);
			break;
		case 2: /* left turn */
			setLED_D11(flasherDivider<ON_TIME_20MS);
			setLED_D1(0);
			break;
		case 3: /* hazard */
			if (flasherDivider>=CYCLE_TIME_HAZARD_20MS) flasherDivider=0;
			setLED_D11(flasherDivider<ON_TIME_HAZARD_20MS);
			setLED_D1(flasherDivider<ON_TIME_HAZARD_20MS);
			break;
		default: /* off */
			setLED_D1(0);
			setLED_D11(0);
	}

}
