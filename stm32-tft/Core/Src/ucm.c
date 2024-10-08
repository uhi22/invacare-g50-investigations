
#include "main.h"
#include "canbus.h"
#include "ucm.h"


uint8_t ucmJoystickX, ucmJoystickY;
uint32_t startupStep;
uint8_t blLightOn;
uint8_t flasherMode;

#define JOYSTICK_LOWER_DEADBAND (2048-250)
#define JOYSTICK_UPPER_DEADBAND (2048+250)

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
	d=-d; /* swap the direction, we want right side to be high value */
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


void runJoystickMain5ms(void) {
	if (startupStep>9000/5) {
		if (startupStep%10 == 0) {
			//runJoystickSimulation50ms();
			convertJoystick();
		}
	}
}
