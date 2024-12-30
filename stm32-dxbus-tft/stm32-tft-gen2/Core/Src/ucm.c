
#include "main.h"
#include "can_lowlayer.h"
#include "can_application.h"
#include "ucm.h"
#include "analogInputs.h"


uint8_t ucmJoystickX, ucmJoystickY;
uint32_t startupStep;
uint8_t ucmError;
uint8_t blLightOn;
uint8_t flasherMode;
uint16_t counterUserWantsToDrive;

#define JOYSTICK_LOWER_DEADBAND (2048-250)
#define JOYSTICK_UPPER_DEADBAND (2048+250)
#define STARTTOLERANCE 4 /* joystick way out of the central 0x80, to change to driving mode. */

void ucm_checkPotNeutralOnStartup(void) {
	if ((startupStep>50) && (startupStep<200)) {
		/* todo: check whether the user input (joystick, poti) is in neutral during startup. */
		if (ucmJoystickX!=0x80) ucmError = UCM_ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_X;
		if (ucmJoystickY!=0x80) ucmError = UCM_ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_Y;
	}
}

void convertJoystick(void) {
	int16_t d;
	uint16_t x,y;
	x = adcValues[ADC_CHANNEL_JOYSTICK_X]; /* The AD value is in range 0 to 4095. */
	if (x<JOYSTICK_LOWER_DEADBAND) {
		d = x-JOYSTICK_LOWER_DEADBAND;
	} else if (x>JOYSTICK_UPPER_DEADBAND) {
		d = x-JOYSTICK_UPPER_DEADBAND;
	} else {
		d=0; /* inside the middle dead band, the output value is neutral */
	}
	/* now the d is in range ~ -2000 to 2000 */
	d=d/14;
	/* now the d is in range ~ -127 to 127 */
	//d=-d; /* swap the direction, we want right side to be high value */
	d+=128; /* add the offset, because the DXBUS wants 0x80 as neutral */
	if (d<1) d=1; /* minimum value on DXBUS is 1 */
	if (d>0xFF) d=0xFF; /* maximum value on DXBUS is 255 */
	ucmJoystickX = d;

	y = adcValues[ADC_CHANNEL_JOYSTICK_Y]; /* The AD value is in range 0 to 4095. */
	if (y<JOYSTICK_LOWER_DEADBAND) {
		d = y-JOYSTICK_LOWER_DEADBAND;
	} else if (y>JOYSTICK_UPPER_DEADBAND) {
		d = y-JOYSTICK_UPPER_DEADBAND;
	} else {
		d=0; /* inside the middle dead band, the output value is neutral */
	}
	/* now the d is in range ~ -2000 to 2000 */
	d=d/14;
	/* now the d is in range ~ -127 to 127 */
	d+=128; /* add the offset, because the DXBUS wants 0x80 as neutral */
	if (d<1) d=1; /* minimum value on DXBUS is 1 */
	if (d>0xFF) d=0xFF; /* maximum value on DXBUS is 255 */
	ucmJoystickY = d;
}

uint8_t get_userWantsToDrive(void) {
	/* The user wants to drive, if one of the joystick values is out of neutral. And this
	 * is prolonged for 2 seconds. */
	if ((ucmJoystickY<0x80-STARTTOLERANCE) || (ucmJoystickY>0x80+STARTTOLERANCE) ||
		(ucmJoystickX<0x80-STARTTOLERANCE) || (ucmJoystickX>0x80+STARTTOLERANCE)) {
		counterUserWantsToDrive=2000/5;
	}
	if (counterUserWantsToDrive>0) {
		return 1; /* user wants to drive or wanted it during the last two seconds. */
	} else {
		return 0; /* user does not want to drive. */
	}
}

void runJoystickSimulation50ms(void) {
  static uint8_t phase=0;
  static uint8_t phaseTimer=0;
  if (phase==0) {
	/* simulate increasing right turn */
	if (ucmJoystickX<0xFE) {
		ucmJoystickX++;
	} else {
		phase++;
	}
  }
  if (phase==1) {
	/* simulate acceleration */
	if (ucmJoystickY<0xFE) {
		ucmJoystickY++;
	} else {
		phase++;
	}
  }
  if (phase==2) {
	/* simulate right-to-left-turn */
	if (ucmJoystickX>0x5) {
		ucmJoystickX-=2;
	} else {
		phase++;
	}
  }
  if (phase==3) {
	/* simulate deceleration and full reverse speed */
	if (ucmJoystickY>0x5) {
		ucmJoystickY-=2;
	} else {
		phase++;
	}
  }
  if (phase==4) {
	/* simulate quick stop */
	if (ucmJoystickY<0x80) {
		ucmJoystickY+=10;
	} else {
		ucmJoystickY=0x80;
		phase++; phaseTimer=0;
	}
  }
  if (phase==5) {
	ucmJoystickX = 0x80;
	ucmJoystickY = 0xA0; /* slow forward */
	phaseTimer++;
	if (phaseTimer>20) {
		phase++; phaseTimer=0;
	}
  }
  if (phase==6) {
	ucmJoystickX = 0x88; /* right */
	ucmJoystickY = 0xFE; /* fast forward */
	phaseTimer++;
	if (phaseTimer>200) {
		phase++; phaseTimer=0;
	}
  }
  if (phase==7) {
	ucmJoystickX = 0x78; /* left */
	ucmJoystickY = 0x01; /* fast backwards */
	phaseTimer++;
	if (phaseTimer>200) {
	    phaseTimer=0;
		phase=6;
	}
  }

}


void ucm_mainfunction5ms(void) {
  ucm_checkPotNeutralOnStartup();
  if (startupStep%10 == 0) {
	  convertJoystick();
  }
  if (counterUserWantsToDrive>0) counterUserWantsToDrive--;
}
