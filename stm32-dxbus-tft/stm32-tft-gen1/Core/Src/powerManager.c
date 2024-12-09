
#include "main.h"
#include "powerManager.h"
#include "hardwareInterface.h"


/* Shutdown timing:

22818ms,000002BC,false,Rx,9,3,03,00,0F last "normal"
22942ms,000002BC,false,Rx,9,3,03,00,01
22997ms,000003A4,false,Rx,9,3,03,00,04
23337ms,000003AC,false,Rx,9,3,03,00,05
This means, the shutdown takes ~400ms.
 */

#define SHUTDOWN_TIME_MS 440

uint8_t pwrM_shutdownTimer;

uint8_t pwrM_isShutdownOngoing(void) {
	return pwrM_shutdownTimer>0;
}

void pwrM_startThePowerOffSequence(void) {
	pwrM_shutdownTimer = SHUTDOWN_TIME_MS/20; /* in 20ms */
}

void pwrM_mainfunction20ms(void) {
	if (pwrM_shutdownTimer>0) {
		pwrM_shutdownTimer--;
		if (pwrM_shutdownTimer==0) {
			 setKeepPowerOn(0); /* debounce is finished -> turn off */
		}
	}
}
