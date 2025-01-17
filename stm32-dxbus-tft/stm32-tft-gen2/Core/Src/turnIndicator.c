
#include "turnIndicator.h"
#include "hardwareAbstraction.h"

uint8_t oldRightButton;
uint8_t oldLeftButton;
uint8_t turnIndicatorMode;
uint8_t flasherDivider;
uint8_t rightAndLeftCounter;
uint8_t flasherLiveState;

#define HAZARD_DEBOUNCE_CYCLES 20 /* 20*5ms = 100ms debounce time for hazard flashing */

uint8_t turni_isRightOn(void) {
  return (flasherLiveState & 1)!=0;
}

uint8_t turni_isLeftOn(void) {
  return (flasherLiveState & 2)!=0;
}


void turni_handleButtons(uint8_t leftbutton, uint8_t rightbutton) { /* runs in 5ms cycle */
	if (rightbutton && leftbutton) {
		/* both turn-buttons are pressed -> user wants hazard flashing */
		if (rightAndLeftCounter<HAZARD_DEBOUNCE_CYCLES) {
			rightAndLeftCounter++;
			if (rightAndLeftCounter==HAZARD_DEBOUNCE_CYCLES) {
				/* user-demand for hazard flashing is debounced -> activate hazard flashing */
				turnIndicatorMode = 3; /* turn on the hazard flashing */
				flasherDivider = 0; /* start with a full on-time */
			}
		}
		oldLeftButton = leftbutton;
		oldRightButton = rightbutton;
		return;
	}
	rightAndLeftCounter=0;
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
    #ifdef USE_HAZARD_BUTTON
	if (hazardButton && !oldHazardButton) {
		if (turnIndicatorMode == 3) {
			turnIndicatorMode = 0; /* if hazard flashing was already running, then turn it off. */
		} else {
			turnIndicatorMode = 3; /* turn on the hazard flashing */
			flasherDivider = 0; /* start with a full on-time */
		}
	}
    #endif
	oldLeftButton = leftbutton;
	oldRightButton = rightbutton;
}

#define CYCLE_TIME_20MS (760/20)
#define ON_TIME_20MS (380/20)
#define CYCLE_TIME_HAZARD_20MS (680/20)
#define ON_TIME_HAZARD_20MS (340/20)

void turni_mainfunction20ms(void) {
	flasherDivider++;
	if (flasherDivider>=CYCLE_TIME_20MS) flasherDivider=0;
	flasherLiveState=0;
	switch (turnIndicatorMode) {
		case 1: /* right turn */
			if (flasherDivider<ON_TIME_20MS) flasherLiveState=1;
			setLED_D1(flasherDivider<ON_TIME_20MS);
			setLED_D11(0);
			setOut1(flasherDivider<ON_TIME_20MS);
			setOut3(0);
			break;
		case 2: /* left turn */
			if (flasherDivider<ON_TIME_20MS) flasherLiveState=2;
			setLED_D11(flasherDivider<ON_TIME_20MS);
			setLED_D1(0);
			setOut3(flasherDivider<ON_TIME_20MS);
			setOut1(0);
			break;
		case 3: /* hazard */
			if (flasherDivider>=CYCLE_TIME_HAZARD_20MS) flasherDivider=0;
			if (flasherDivider<ON_TIME_HAZARD_20MS) flasherLiveState=3;
			setLED_D11(flasherDivider<ON_TIME_HAZARD_20MS);
			setLED_D1(flasherDivider<ON_TIME_HAZARD_20MS);
			setOut1(flasherDivider<ON_TIME_HAZARD_20MS);
			setOut3(flasherDivider<ON_TIME_HAZARD_20MS);
			break;
		default: /* off */
			setLED_D1(0);
			setLED_D11(0);
			setOut1(0);
			setOut3(0);
	}

}
