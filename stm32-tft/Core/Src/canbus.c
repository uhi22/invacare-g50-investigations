
#include "main.h"
#include "canbus.h"

/* Evaluation of DXBUS messages
*/
#define DIVIDER_STOPPED 0xFFFF

uint32_t canTime5ms;
uint32_t startupStep;

uint16_t divider29c = DIVIDER_STOPPED;

uint8_t ucmJoystickX, ucmJoystickY;
uint8_t ucmState, motorState, servoLightState;
uint8_t motorUBattRaw;
uint8_t ucmLightDemand;
uint32_t canTxErrorCounter, canTxOkCounter;
uint8_t ucmOwnState=0x10;
uint8_t ucmOwnJoystickX, ucmOwnJoystickY;

#define MESSAGE_ID_UCM        0x040 /* The UCM (user control module) which is joystick and keys */
#define MESSAGE_ID_MOTOR      0x008 /* The motor controller */
#define MESSAGE_ID_SERVOLIGHT 0x010 /* The servo and lighting module */

#define USE_ACTIVE_CONTROL


void setDebugPin(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	}
}

void setWakeupOutput(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	}
}

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
 #ifdef USE_ACTIVE_CONTROL
	 TxHeader.StdId = canId;
	 TxHeader.DLC = length;
	 if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan)>0) {
		 if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
			canTxErrorCounter++; /* todo: recovery */
		 } else {
			canTxOkCounter++;
		 }
	 } else {
		 /* no free mailbox */
		 canTxErrorCounter++;
	 }
#endif
}


void handle29C(void) {
	/* The 29C comes each 120ms. */
	if (divider29c!=DIVIDER_STOPPED) {
		divider29c++;
		if (divider29c>=120/5) {
			divider29c=0;
			TxData[0] = 0x03; TxData[1] = 0x00; TxData[2] = 0x1A;
			//tryToTransmit(0x29C, 3);
		}
	}
}

void can_mainfunction5ms(void) {
	canTime5ms++;
	startupStep++;

	if ((canTime5ms%4)==0) {
		TxData[0] = 0xAA;
		TxData[1] = 0x04;
		tryToTransmit(0x0AA, 2);
	} else if ((canTime5ms%4)==2) {
		if (ucmOwnState>=0x20) {
			TxData[0] = 0xB0;
			TxData[1] = 0x01;
			TxData[2] = ucmOwnState; /* starts with 0x10, then changes to 0x20. */
			TxData[3] = 0x90;
			TxData[4] = ucmOwnJoystickY;
			TxData[5] = ucmOwnJoystickX;
			tryToTransmit(0x040, 6);
		} else {
			TxData[0] = 0xB0;
			TxData[1] = 0x01;
			TxData[2] = ucmOwnState; /* starts with 0x10, then changes to 0x20. */
			tryToTransmit(0x040, 3);
		}
	}
	setDebugPin(startupStep & 1);
	if (startupStep<10) {
	  /* does it make sense to send anything before the wakeup??? */
	}
	if (startupStep==20) {
		/* The wakeup pulse. Set output to high, to send >9V to the CANH, for 40ms. This
		 * will wake up the other control units.
		 */
		setWakeupOutput(1);
	}
	if (startupStep==20+40/5) {
		/* end of the wakeup pulse */
		setWakeupOutput(0);
	}
	if ((startupStep==20+40/5+2) || (startupStep==20+40/5+3)) {
		/* The 23 80 00 08 FC 80 00 00 comes two times with 5ms in between */
		TxData[0] = 0x23; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
		TxData[4] = 0xFC; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
		tryToTransmit(0x040, 8);
	}
	if ((startupStep==20+40/5+2+2) || (startupStep==20+40/5+2+3)) {
		/* The 21 80 00 08 FC 80 00 00 comes two times with 5ms in between */
		TxData[0] = 0x21; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
		TxData[4] = 0xFC; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
		tryToTransmit(0x040, 8);
	}
	if ((startupStep==20+40/5+2+2+2) || (startupStep==20+40/5+2+2+3)) {
		/* The 24 80 00 08 FC 40 00 00 comes two times with 5ms in between */
		TxData[0] = 0x24; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0x08;
		TxData[4] = 0xFC; TxData[5] = 0x40; TxData[6] = 0x00; TxData[7] = 0x00;
		tryToTransmit(0x040, 8);
	}
	if (startupStep==20+40/5+2+2+2 + 130/5) {
		/* after a gap of 130ms, the 27,00,00,FD,00,80,00,00 comes once */
		TxData[0] = 0x27; TxData[1] = 0x00; TxData[2] = 0x00; TxData[3] = 0xFD;
		TxData[4] = 0x00; TxData[5] = 0x80; TxData[6] = 0x00; TxData[7] = 0x00;
		tryToTransmit(0x040, 8);
	}
	if (startupStep==20+40/5+2+2+2 + 140/5) {
		/* 13,01,00,00,00,FF,00,00, comes once */
		TxData[0] = 0x13; TxData[1] = 0x01; TxData[2] = 0x00; TxData[3] = 0x00;
		TxData[4] = 0x00; TxData[5] = 0xFF; TxData[6] = 0x00; TxData[7] = 0x00;
		tryToTransmit(0x040, 8);
	}
	if (startupStep==20+40/5+2+2+2 + 150/5) {
		/* here the 0x29C starts and runs each 120ms */
		divider29c = 120/5; /* "expired", immediately send the message */
	}

	if (startupStep>1000/5) {
		if ((canTime5ms%4)==0) {
		  /* Demo for light control */
		  uint16_t lightDivider = canTime5ms%400;
		  uint8_t lightControl = 0x00; /* 0 is lightsOff */
		  if (lightDivider<100) {
			  lightControl = 0x11; /* low beam */
		  } else if (lightDivider<200) {
			  lightControl = 0x22; /* right turn */
		  } else if (lightDivider<300) {
			  lightControl = 0x44; /* left turn */
		  }
		  /* For controlling the low beam lights, the original message is
		     0x040 B0 93 E1 00 14 11
		     This contains two network variables, the nr 93 and the nr 14.
		    TxData[0] = 0xB0; TxData[1] = 0x93; TxData[2] = 0xE1; TxData[3] = 0x00;
		    TxData[4] = 0x14; TxData[5] = lightControl;
		    To test, whether the nr 93 is relevant for the lights, we just omit it,
		    and use the short version which only contains the network variable nr 14:
		    0x040 B0 14 11.
		    Result: Also this works perfectly for controlling the lights. */
		  TxData[0] = 0xB0; TxData[1] = 0x14; TxData[2] = lightControl;
		  tryToTransmit(0x040, 3);

		}
	}

	if (startupStep==10000/5) {
		if (ucmOwnState==0x10) ucmOwnState = 0x20;
		ucmOwnJoystickY = 0x85;
		ucmOwnJoystickX = 0x83;
	}
	if (startupStep==15000/5) {
		if (ucmOwnState==0x20) ucmOwnState = 0x23;
	}
	if (startupStep==17000/5) {
		if (ucmOwnState==0x23) ucmOwnState = 0x24;
	}
	if (startupStep==17020/5) {
		if (ucmOwnState==0x24) ucmOwnState = 0x25;
	}
	if (startupStep>17000/5) {
		if (startupStep%10 == 0) {
			if (ucmOwnJoystickX<0xF0) ucmOwnJoystickX++;
		}
	}

	handle29C();
}

void can_init(void) {
	TxHeader.IDE = CAN_ID_STD; /* standard ID (not extended ID) */
	TxHeader.RTR = CAN_RTR_DATA; /* data message (not remote request message) */
}
