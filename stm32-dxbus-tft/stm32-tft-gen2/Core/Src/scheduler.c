
#include "hardwareAbstraction.h"
#include "analogInputs.h"
#include "buttons.h"
#include "powerManager.h"
#include "turnIndicator.h"
#include "drivingLight.h"
#include "display.h"
#include "can_lowlayer.h"
#include "can_application.h"
#include "ucm.h"
#include "slm.h"
#include "drivepedal.h"


//extern void runJoystickMain5ms(void);

uint16_t nMainLoops;
uint32_t oldTime20ms;
uint32_t oldTime5ms;
uint32_t counter5ms;


void task5ms(void) {
	setOutTP1(1);
	ADC_cyclicScan();
	ucm_mainfunction5ms();
	can_mainfunction5ms();
	buttons_mainfunction();
	slm_mainfunction5ms();
	counter5ms++;
	setLEDAlive((counter5ms & 0x20)!=0);
	setOutTP1(0);
}

void task20ms(void) {
  display_mainfunction20ms();
  pwrM_mainfunction20ms();
  turni_mainfunction20ms();
  light_mainfunction20ms();
  drivepedal_mainfunction20ms();
}


void scheduler_init(void) {
	oldTime20ms = HAL_GetTick();
	oldTime5ms = oldTime20ms;
}

void scheduler_cyclic(void) {
	nMainLoops++;
	uint32_t t;
	t = HAL_GetTick();
	if (t>=oldTime20ms+20) {
		oldTime20ms+=20;
		task20ms();
	}
	if (t>=oldTime5ms+5) {
		oldTime5ms+=5;
		task5ms();
	}
}
