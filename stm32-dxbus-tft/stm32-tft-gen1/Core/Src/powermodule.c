
/* Simulation of the power module (aka motor control unit) */
/* The simulation of the PowerModule is necessary for the case that we want to control the ServoLightModule without
   using a real PowerModule. */

#define SIMULATE_POWER_MODULE

#include "main.h"
#include "canbus.h"
#include "powermodule.h"
#include "ucm.h"

uint8_t powermodule_cycledivider;
uint8_t powermodule_timer_040_23;
uint8_t powermodule_timer_040_21;
uint8_t powermodule_timer_040_24;
uint8_t powermoduleState;
uint8_t nWaitUcmStartup=5;

#ifdef SIMULATE_POWER_MODULE
void powermodule_runStatemachine(void) {
    if (nWaitUcmStartup>0) {
    	nWaitUcmStartup--;
        return;
    }
    if (ucmOwnState == 0x10) powermoduleState = 0x10;
    if (ucmOwnState == 0x20) powermoduleState = 0x20;
    if ((ucmOwnState == 0x23) && (powermoduleState==0x20)) powermoduleState = 0x23;
    if ((ucmOwnState == 0x23) && (powermoduleState==0x23)) powermoduleState = 0x25;
}
#endif

void powermodule_init(void) {
    #ifdef SIMULATE_POWER_MODULE
    powermoduleState=0x02; /* initial state is 0x02 */
    #endif
}

/* During startup, if the UCM sends the "23", the powermodule needs to send the "A3" response, so that the SLM knows
   that the motor is present. If the the "A3" from the powermodule is missing, the SLM is also silent.
   Good case:
    6909649,00000040,false,Rx,9,8,23,00,00,08,FC,80,00,00,
    6910411,00000008,false,Rx,9,1,A3,00,00,00,00,00,00,00,
    6910967,00000010,false,Rx,9,1,A3,00,00,00,00,00,00,00,
*/

void powermodule_informAboutCanMessage(uint16_t id, uint8_t firstbyte) {
    if (id==0x40) {
    	if (firstbyte==0x23) powermodule_timer_040_23 = 1; /* we want to send a response */
    	if (firstbyte==0x21) powermodule_timer_040_21 = 1; /* we want to send a response */
    	if (firstbyte==0x24) powermodule_timer_040_24 = 1; /* we want to send a response */
    }
}

void powermodule_mainfunction5ms(void) {
    #ifdef SIMULATE_POWER_MODULE
    powermodule_cycledivider++;
    powermodule_runStatemachine();
    if (powermodule_timer_040_23>0) {
        powermodule_timer_040_23--;
        if (powermodule_timer_040_23==0) {
            TxData[0] = 0xA3;
			TxData[1] = 0;
			tryToTransmit(0x08, 1);
        }
    }
    if (powermodule_timer_040_21>0) {
        powermodule_timer_040_21--;
        if (powermodule_timer_040_21==0) {
            TxData[0] = 0xA1;
			TxData[1] = 0;
			tryToTransmit(0x08, 1);
        }
    }
    if (powermodule_timer_040_24>0) {
        powermodule_timer_040_24--;
        if (powermodule_timer_040_24==0) {
            TxData[0] = 0xA4;
			TxData[1] = 0;
			tryToTransmit(0x08, 1);
        }
    }
    /* announce the state of the power module */
    if ((powermodule_cycledivider % 16)==0) {
        	TxData[0] = 0xB0;
			TxData[1] = 0x01; TxData[2] = powermoduleState;
			tryToTransmit(0x08, 3);
    }
    /* announce the network variables 0F, 0E and 07 to make the SLM happy */
    if ((powermodule_cycledivider % 16)==1) {
        	TxData[0] = 0xB0;
			TxData[1] = 0x0F; TxData[2] = 0xFF;
			TxData[3] = 0x0E; TxData[4] = 0x00;
			TxData[5] = 0x07; TxData[6] = 0x00;
			tryToTransmit(0x08, 7);
    }
    #endif
}
