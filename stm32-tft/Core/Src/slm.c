/* ServoLightModule */
/* Simulates the ServoLightModule */

#define EMULATE_SLM

#include "main.h"
#include "canbus.h"
#include "slm.h"

uint8_t slm_cycleDivider;
uint8_t slm_slowDivider;
uint8_t servoLightState=0x10;


void slm_transmitSlowMessages(void) {
    slm_slowDivider++;
    switch (slm_slowDivider%4) {
        case 0:
            TxData[0] = 0xB0;
            TxData[1] = 0x0E; TxData[2] = 00; /* maybe error flags? */
            TxData[3] = 0x0B; TxData[4] = 00; /* maybe servo position? */
            tryToTransmit(0x010, 5);
            break;
        case 1:
        	/* todo: send the profile requests */
        	break;
    }
}

void slmSim_runStateMachine(void) {
    switch (servoLightState) {
        case 0x10: /* the initial state */
            if (ucmState==0x20) servoLightState=0x20; /* The SLM follows the UCM, and reaches "idle". */
            break;
        case 0x20: /* idle */
            if (ucmState==0x23) servoLightState=0x23; /* The SLM follows the UCM. */
            break;
        case 0x23: /* start driving */
            if (motorState==0x25) servoLightState=0x25; /* The SLM follows the motor, and reaches "driving". */
            break;
        case 0x25: /* driving */
            if (ucmState==0x22) servoLightState=0x21; /* The UCM wants to stop. We go to 21 as intermediate step. */
            break;
        case 0x21: /* stopping1 */
            if (motorState==0x22) servoLightState=0x22; /* The motor confirmed the stop. We follow this. */
            break;
        case 0x22: /* stopping2 */
            if (motorState==0x20) servoLightState=0x20; /* The motor confirmed the stop2. We follow this. */
            break;
    }
}

void slm_mainfunction5ms(void) {
    #ifdef EMULATE_SLM
    slm_cycleDivider++;
    switch (slm_cycleDivider%4) {
    	case 0:
			slmSim_runStateMachine();
			/* send the SLM specific CAN messages */
			/* 20ms cycle, slot 0 */
			TxData[0] = 0xB0;
			TxData[1] = 0x01; TxData[2] = servoLightState;
			TxData[3] = 0x11; TxData[4] = ucmState;
			TxData[5] = 0x0D; TxData[6] = 0x00; /* network variable 0D is always 0x00 */
			tryToTransmit(0x010, 5);
			break;
    	case 1:
    		/* 20ms cycle, slot 1 */
			TxData[0] = 0xB0;
			TxData[1] = 0x0B; TxData[2] = 00; /* Each 20ms. Maybe servo position? Or speed??? */
			tryToTransmit(0x010, 3);
			break;
    	case 2:
    		/* slot 2 */
    		slm_transmitSlowMessages();
    		break;
    }
    #endif
}
