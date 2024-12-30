/* ServoLightModule */
/* Simulates the ServoLightModule */

#define EMULATE_SLM

#include "main.h"
#include "can_lowlayer.h"
#include "can_application.h"
#include "ucm.h"
#include "slm.h"

uint8_t slm_cycleDivider;
uint8_t slm_slowDivider;
uint8_t servoLightState=0x10;
int8_t slm_outMotorSpeedRequest=0;
uint16_t paramRequestState=0;
float filteredUserRequest;
float signedUserRequest;
uint8_t creepMode = 1;


void slm_convertUserInputIntoMotorSpeed(void) {
    if (ucmError == 0) {
        /* no error. Take the users wish and give it to the motor. */
		#ifdef DIRECTLY_UNFILTERED
          slm_outMotorSpeedRequest = ucmJoystickY-0x80;
        #else
          signedUserRequest = ucmJoystickY-0x80;
          if (creepMode) signedUserRequest = signedUserRequest / 4;
          filteredUserRequest = filteredUserRequest * 0.998 + signedUserRequest * 0.002;
          if (filteredUserRequest>127) filteredUserRequest = 127;
          if (filteredUserRequest<-127) filteredUserRequest = -127;
          slm_outMotorSpeedRequest = (int8_t)filteredUserRequest;
        #endif
    } else {
        /* if we have an error (joystick error, poti error, ...) then ramp down the motor speed to zero. */
        if (slm_outMotorSpeedRequest>0) slm_outMotorSpeedRequest--;
        if (slm_outMotorSpeedRequest<0) slm_outMotorSpeedRequest++;
    }
}

void slm_handleParameterRequests5ms(void) {
	/* The physical servoLightModule requests the network parameters 2C to 35 from
	 * the UCM. These parameters are not relevant for the simulated SLM. Also
	 * the motor control module works fine without the parameter transfer.
	 * Originally, each variable is requested for ~250ms.
	 */
	#define N_START (200/5)
	#define N_DELTA (250/5)
	/*
	if (paramRequestState<1000) paramRequestState++;
	if (paramRequestState==N_START) { isSubscribedNv2C=1; }
	if (paramRequestState==N_START+1*N_DELTA) { isSubscribedNv2C=0; isSubscribedNv2D=1; }
	if (paramRequestState==N_START+2*N_DELTA) { isSubscribedNv2D=0; isSubscribedNv2E=1; }
	if (paramRequestState==N_START+3*N_DELTA) { isSubscribedNv2E=0; isSubscribedNv2F=1; }
	if (paramRequestState==N_START+4*N_DELTA) { isSubscribedNv2F=0; isSubscribedNv30=1; }
	if (paramRequestState==N_START+5*N_DELTA) { isSubscribedNv30=0; isSubscribedNv31=1; }
	if (paramRequestState==N_START+6*N_DELTA) { isSubscribedNv31=0; isSubscribedNv32=1; }
	if (paramRequestState==N_START+7*N_DELTA) { isSubscribedNv32=0; isSubscribedNv33=1; }
	if (paramRequestState==N_START+8*N_DELTA) { isSubscribedNv33=0; isSubscribedNv34=1; }
	if (paramRequestState==N_START+9*N_DELTA) { isSubscribedNv34=0; isSubscribedNv35=1; }
	if (paramRequestState==N_START+10*N_DELTA) { isSubscribedNv35=0; }
	*/
}

void slm_transmitSlowMessages(void) {
    slm_slowDivider++;
    switch (slm_slowDivider%4) {
        case 0:
            TxData[0] = 0xB0;
            TxData[1] = 0x0E; TxData[2] = 00; /* maybe error flags? */
            tryToTransmit(0x010, 3);
            break;
        case 1:
        	break;
    }
}

void slmSim_runStateMachine(void) {
    switch (servoLightState) {
        case 0x10: /* the initial state */
            if (ucmOwnState==0x20) servoLightState=0x20; /* The SLM follows the UCM, and reaches "idle". */
            break;
        case 0x20: /* idle */
            if (ucmOwnState==0x23) servoLightState=0x23; /* The SLM follows the UCM. */
            break;
        case 0x23: /* start driving */
            if (powermoduleState==0x25) servoLightState=0x25; /* The SLM follows the motor, and reaches "driving". */
            break;
        case 0x25: /* driving */
            if (ucmOwnState==0x22) servoLightState=0x21; /* The UCM wants to stop. We go to 21 as intermediate step. */
            break;
        case 0x21: /* stopping1 */
            if (powermoduleState==0x22) servoLightState=0x22; /* The motor confirmed the stop. We follow this. */
            break;
        case 0x22: /* stopping2 */
            if (powermoduleState==0x20) servoLightState=0x20; /* The motor confirmed the stop2. We follow this. */
            break;
    }
}

void slm_mainfunction5ms(void) {
    #ifdef EMULATE_SLM
	slm_handleParameterRequests5ms();
	slm_convertUserInputIntoMotorSpeed();
    slm_cycleDivider++;
    switch (slm_cycleDivider%4) {
    	case 0:
			slmSim_runStateMachine();
			/* send the SLM specific CAN messages */
			/* 20ms cycle, slot 0 */
			TxData[0] = 0xB0;
			TxData[1] = 0x01; TxData[2] = servoLightState;
			TxData[3] = 0x11; TxData[4] = ucmOwnState;
			TxData[5] = 0x0D; TxData[6] = 0x00; /* network variable 0D is always 0x00 */
			setOutTP21(blTP21toggle);blTP21toggle=1-blTP21toggle;
			tryToTransmit(0x010, 7);
			break;
    	case 2:
    		/* 20ms cycle, slot 1 */
			TxData[0] = 0xB0;
			TxData[1] = 0x0B; TxData[2] = slm_outMotorSpeedRequest; /* Each 20ms. Motor speed request. */
			tryToTransmit(0x010, 3);
			break;
    	case 3:
    		/* slot 2 */
    		slm_transmitSlowMessages();
    		break;
    }
    #endif
}
