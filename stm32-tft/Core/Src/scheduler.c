
#include "main.h"
#include "buttons.h"
#include "powerManager.h"


extern void display_update20ms(void);
extern void runJoystickMain5ms(void);
extern void can_mainfunction5ms(void);

uint16_t nMainLoops;
uint32_t oldTime20ms;
uint32_t oldTime5ms;


void task5ms(void) {
	runJoystickMain5ms();
	can_mainfunction5ms();
	buttons_mainfunction();
}

void task20ms(void) {
  display_update20ms();
  pwrM_mainfunction20ms();
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
