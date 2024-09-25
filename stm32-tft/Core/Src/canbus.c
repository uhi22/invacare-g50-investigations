
#include "main.h"
#include "canbus.h"

/* Evaluation of DXBUS messages
*/
#define DIVIDER_STOPPED 0xFFFF

uint32_t canTime10ms;
uint32_t startupStep;

uint16_t divider29c = DIVIDER_STOPPED;

uint8_t ucmJoystickX, ucmJoystickY;
uint8_t ucmState, motorState, servoLightState;
uint8_t motorUBattRaw;
uint8_t ucmLightDemand;
uint32_t canTxErrorCounter, canTxOkCounter;

#define MESSAGE_ID_UCM        0x040 /* The UCM (user control module) which is joystick and keys */
#define MESSAGE_ID_MOTOR      0x008 /* The motor controller */
#define MESSAGE_ID_SERVOLIGHT 0x010 /* The servo and lighting module */


void canEvaluateReceivedMessage(void) {
    /* This is called in interrupt context. Keep it as short as possible. */
	/* Todo: check for message length. */
	/* Todo: more general parsing of the network variables */
    if (canRxMsgHdr.StdId == MESSAGE_ID_UCM) {
    	if (canRxData[0]==0xB0) {
    		if (canRxData[1]==0x01) {
    			ucmState = canRxData[2];
    		}
    		if (canRxData[3]==0x90) {
    			ucmJoystickY = canRxData[4];
    			ucmJoystickX = canRxData[5];
    		}
    		if (canRxData[1]==0x93) {
        		if (canRxData[4]==0x14) {
        			ucmLightDemand = canRxData[5];
        		}
    		}
    	}
        return;
    }
    if (canRxMsgHdr.StdId == MESSAGE_ID_MOTOR) {
    	if (canRxData[0]==0xB0) {
    		if (canRxData[1]==0x01) {
    			motorState = canRxData[2];
    		}
    		if (canRxData[3]==0x0C) {
    			motorUBattRaw = canRxData[4];
    		}
    	}
        return;
    }
    if (canRxMsgHdr.StdId == MESSAGE_ID_SERVOLIGHT) {
    	if (canRxData[0]==0xB0) {
    		if (canRxData[1]==0x01) {
    			servoLightState = canRxData[2];
    		}
    	}
        return;
    }

}

void tryToTransmit(uint16_t canId, uint8_t length) {
 if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
	canTxErrorCounter++; /* todo: recovery */
 } else {
	canTxOkCounter++;
 }
}


void handle29C(void) {
	/* The 29C comes each 120ms. */
	if (divider29c!=DIVIDER_STOPPED) {
		divider29c++;
		if (divider29c>=12) {
			divider29c=0;
			TxData[0] = 0x03; TxData[1] = 0x00; TxData[2] = 0x1A;
			tryToTransmit(0x29C, 3);
		}
	}
}

void can_mainfunction10ms(void) {
	canTime10ms++;

	if ((canTime10ms%2)==0) {
		TxData[0] = 0xAA;
		TxData[1] = 0x04;
		tryToTransmit(0x0AA, 2);
	} else {
		TxData[0] = 0xB0;
		TxData[1] = 0x01;
		TxData[2] = 0x10;
		tryToTransmit(0x040, 3);
	}
	if (canTime10ms>10) {
		if (startupStep<=1) {
			/* The 23 80 00 08 FC 80 00 00 comes two times with 5ms in between */
			TxData[0] = 0x23; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
			TxData[4] = 0xFC; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
			tryToTransmit(0x040, 8);
		} else if (startupStep<=3) {
			/* The 21 80 00 08 FC 80 00 00 comes two times with 5ms in between */
			TxData[0] = 0x21; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
			TxData[4] = 0xFC; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
			tryToTransmit(0x040, 8);
		} if (startupStep<=5) {
			/* The 24 80 00 08 FC 40 00 00 comes two times with 5ms in between */
			TxData[0] = 0x24; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
			TxData[4] = 0xFC; TxData[5] = 0x40; TxData[6] = 0x00; TxData[7] = 0x00;
			tryToTransmit(0x040, 8);
		}
		if (startupStep==5+13) {
			/* after a gap of 130ms, the 27,00,00,FD,00,80,00,00 comes once */
			TxData[0] = 0x27; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0xFD;
			TxData[4] = 0x00; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
			tryToTransmit(0x040, 8);
		}
		if (startupStep==5+13+1) {
			/* 13,01,00,00,00,FF,00,00, comes once */
			TxData[0] = 0x13; TxData[1] = 0x01; TxData[2] = 0x00; TxData[3] = 0x00;
			TxData[4] = 0x00; TxData[5] = 0xFF; TxData[6] = 0x00; TxData[7] = 0x00;
			tryToTransmit(0x040, 8);
		}
		if (startupStep==5+13+3) {
			/* here the 0x29C starts and runs each 120ms */
			divider29c = 12; /* "expired", immediately send the message */
		}
		startupStep++;
	}
	handle29C();
}

void can_init(void) {
	TxHeader.IDE = CAN_ID_STD; /* standard ID (not extended ID) */
	TxHeader.RTR = CAN_RTR_DATA; /* data message (not remote request message) */
}
