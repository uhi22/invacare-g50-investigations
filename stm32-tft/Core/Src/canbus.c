
#include "main.h"
#include "canbus.h"

/* Evaluation of DXBUS messages
*/

uint8_t ucmJoystickX, ucmJoystickY;
uint8_t ucmState, motorState, servoLightState;
uint8_t motorUBattRaw;
uint8_t ucmLightDemand;
uint32_t canTxErrorCounter, canTxOkCounter;

#define MESSAGE_ID_UCM        0x040 /* The UCM (user control module) which is joystick and keys */
#define MESSAGE_ID_MOTOR      0x008 /* The motor controller */
#define MESSAGE_ID_SERVOLIGHT 0x010 /* The servo and lighting module */

extern CAN_HandleTypeDef hcan;
CAN_RxHeaderTypeDef canRxMsgHdr;
uint8_t canRxData[8];

extern CAN_TxHeaderTypeDef   TxHeader;
extern uint8_t               TxData[8];
extern uint32_t              TxMailbox;

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


void can_mainfunction20ms(void) {
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.StdId = 0x040;
	TxHeader.DLC = 3;
	TxData[0] = 0xB0;
	TxData[1] = 0x01;
	TxData[2] = 0x10;
	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
		canTxErrorCounter++;
	} else {
		canTxOkCounter++;
	}

	TxHeader.StdId = 0x0AA;
	TxHeader.DLC = 2;
	TxData[0] = 0xAA;
	TxData[1] = 0x04;
	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
		canTxErrorCounter++;
	} else {
		canTxOkCounter++;
	}

}
