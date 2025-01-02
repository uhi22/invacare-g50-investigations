
#include "main.h"
#include "string.h" /* for memcpy */
#include "hardwareAbstraction.h"
#include "can_application.h"
#include "can_lowlayer.h"
#include "ucm.h"
#include "powerManager.h"
#include "errors.h"

uint8_t undervoltageDebounceCounter;
uint8_t goodvoltageDebounceCounter;

/* Evaluation of DXBUS messages
*/
#define DIVIDER_STOPPED 0xFFFF

uint8_t profileNumber = 5; /* 1 to 5 */

uint8_t profileData[5*10] = {
		/* profiles:
		  1     2     3     4     5  */
		0x10, 0x59, 0x66, 0x73, 0xff, /* NV 0x2C */ /* max speed fwd. 10=very low, ff=high speed */
		0x66, 0x66, 0x73, 0x80, 0x80, /* NV 0x2D */
		0x20, 0x80, 0x80, 0x80, 0x80, /* NV 0x2E */
		0x10, 0x66, 0x66, 0x66, 0xff, /* NV 0x2F */ /* max speed reverse. 10=very low, ff=high speed */
		0x66, 0x66, 0x73, 0x80, 0x80, /* NV 0x30 */
		0x99, 0x99, 0x99, 0x99, 0x99, /* NV 0x31 */
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, /* NV 0x32 */
		0xB3, 0xB3, 0xB3, 0xB3, 0xB3, /* NV 0x33 */
		0x33, 0x33, 0x4D, 0x4D, 0x4D, /* NV 0x34 */
		0x80, 0x80, 0x99, 0xA6, 0xA6, /* NV 0x35 */
};

#define NV_SOURCE_UCM 0
#define NV_SOURCE_MOTOR 1
#define NV_SOURCE_SERVOLIGHT 2

uint32_t canTime5ms;
extern uint32_t startupStep;


uint16_t divider120ms = DIVIDER_STOPPED;

uint8_t powermoduleState;
uint8_t motorUBattRaw;
uint8_t ucmLightDemand;
uint32_t canTxErrorCounter, canTxOkCounter;
uint8_t ucmOwnState=0x10; /* 0x10 is the initial state */
int8_t servoPosition;
float UBatt_V;

uint8_t isSubscribedNv2C;
uint8_t isSubscribedNv2D;
uint8_t isSubscribedNv2E;
uint8_t isSubscribedNv2F;
uint8_t isSubscribedNv30;
uint8_t isSubscribedNv31;
uint8_t isSubscribedNv32;
uint8_t isSubscribedNv33;
uint8_t isSubscribedNv34;
uint8_t isSubscribedNv35;
uint8_t numberOf2BC;
uint8_t nCounterState24DuringStopping;

#define MESSAGE_ID_UCM        0x040 /* The UCM (user control module) which is joystick and keys */
#define MESSAGE_ID_MOTOR      0x008 /* The motor controller */
#define MESSAGE_ID_SERVOLIGHT 0x010 /* The servo and lighting module */




void translateBatteryVoltage(void) {
  /* translates the battery voltage, which comes from the PowerModule in network variable 0x0C,
   * into a human-readable voltage.
   * And compares against the undervoltage threshold.
   */
	/* 150=19.8V, 200=26.5V */
	UBatt_V = ((float)motorUBattRaw - 150.0) * (26.5-19.8)/(200-150) + 19.8;
	if (UBatt_V>=18.0) {
		undervoltageDebounceCounter = 0;
		goodvoltageDebounceCounter++;
		if (goodvoltageDebounceCounter>20) {
			if (globalError == ERR_UNDERVOLTAGE) globalError = ERR_OK;
		}
    } else {
	   undervoltageDebounceCounter++;
	   goodvoltageDebounceCounter=0;
	   if (undervoltageDebounceCounter>20) {
		  globalError = ERR_UNDERVOLTAGE;
	   }
    }
}


void setDebugPin(uint8_t on) {
}

void setWakeupOutput(uint8_t on) {
	if (on) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	}
}

uint8_t getNvLength(uint8_t sourceNode, uint8_t networkVariableId) {
	if (networkVariableId==0x01) return 1;
	return 0;
}

void distributeNvValue(uint8_t sourceNode, uint8_t nvId, uint16_t value) {
 switch (sourceNode) {
	 case NV_SOURCE_UCM:
		 break;
 }
}

void decodeNetworkVariables(uint8_t sourceNode) {
 uint8_t index, nvID, nvLength;
 uint16_t nvValue;
 index = 1;
 nvID = canRxData[index];
 nvLength = getNvLength(sourceNode, nvID);
 if (nvLength==1) { nvValue=canRxData[index+1]; index+=2; }
 if (nvLength==2) { nvValue=(((uint16_t)canRxData[index+1])<<8) + canRxData[index+2]; index+=3; }
 if (nvLength!=0) {
	 distributeNvValue(sourceNode, nvID, nvValue);
 }
}

void canEvaluateReceivedMessage(void) {
    /* This is called in interrupt context. Keep it as short as possible. */
	/* Todo: check for message length. */
	/* Todo: more general parsing of the network variables */
    if (canRxMsgHdr.StdId == MESSAGE_ID_UCM) {
    	if (canRxData[0]==0xB0) {
    		decodeNetworkVariables(NV_SOURCE_UCM);
    		if (canRxData[1]==0x01) {
    			ucmOwnState = canRxData[2];
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
    		decodeNetworkVariables(NV_SOURCE_MOTOR);
    		if (canRxData[1]==0x01) {
    			powermoduleState = canRxData[2];
    		}
    		if (canRxData[3]==0x0C) {
    			motorUBattRaw = canRxData[4];
    			translateBatteryVoltage();
    		}
    	}
        return;
    }
    if (canRxMsgHdr.StdId == MESSAGE_ID_SERVOLIGHT) {
    	if (canRxData[0]==0xB0) {
    		decodeNetworkVariables(NV_SOURCE_SERVOLIGHT);
    		if (canRxData[1]==0x01) {
    			servoLightState = canRxData[2];
    		}
    		if (canRxData[1]==0x0B) {
    			servoPosition = canRxData[2];
    		}
    	}
    	if ((canRxData[0]==0x30) && (canRxData[1]==0x08)) {
    		/* it is a "I wanna know" from the ServoLightModule, and we (UCM, 8) are addressed. */
    		/* canRxData[2] seems to be always 0. Do not care. */
    		/* canRxData[3] is the requested network variable */
    		if (canRxData[3] == 0x2C) isSubscribedNv2C=1;
    		if (canRxData[3] == 0x2D) isSubscribedNv2D=1;
    		if (canRxData[3] == 0x2E) isSubscribedNv2E=1;
    		if (canRxData[3] == 0x2F) isSubscribedNv2F=1;
    		if (canRxData[3] == 0x30) isSubscribedNv30=1;
    		if (canRxData[3] == 0x31) isSubscribedNv31=1;
    		if (canRxData[3] == 0x32) isSubscribedNv32=1;
    		if (canRxData[3] == 0x33) isSubscribedNv33=1;
    		if (canRxData[3] == 0x34) isSubscribedNv34=1;
    		if (canRxData[3] == 0x35) isSubscribedNv35=1;
    	}
    	if ((canRxData[0]==0x31) && (canRxData[1]==0x08)) {
    		/* it is a "unsubscribe" from the ServoLightModule, and we (UCM, 8) are addressed. */
    		/* canRxData[2] seems to be always 0. Do not care. */
    		/* canRxData[3] is the unsubscribed network variable */
    		if (canRxData[3] == 0x2C) isSubscribedNv2C=0;
    		if (canRxData[3] == 0x2D) isSubscribedNv2D=0;
    		if (canRxData[3] == 0x2E) isSubscribedNv2E=0;
    		if (canRxData[3] == 0x2F) isSubscribedNv2F=0;
    		if (canRxData[3] == 0x30) isSubscribedNv30=0;
    		if (canRxData[3] == 0x31) isSubscribedNv31=0;
    		if (canRxData[3] == 0x32) isSubscribedNv32=0;
    		if (canRxData[3] == 0x33) isSubscribedNv33=0;
    		if (canRxData[3] == 0x34) isSubscribedNv34=0;
    		if (canRxData[3] == 0x35) isSubscribedNv35=0;
    	}
        return;
    }

}


uint8_t getProfileData(uint8_t n) {
	/* n is the row in the parameter table, e.g.
	 * n=0 means network variable 2C
	 * n=1 means network variable 2D and so on.
	 */
	return profileData[n*5+(profileNumber-1)];
}

void handleTransmissionOfSubscribedNVs(void) {
  /* we come here each 20ms */
  static uint8_t subscriptionDivider;
  subscriptionDivider++; if (subscriptionDivider==10) subscriptionDivider=0; /* create a 200ms cycle */
  TxData[0] = 0xB0; /* announcement of network variable */
  switch (subscriptionDivider) {
  	  case 0:
  		  if (isSubscribedNv2C) { TxData[1] = 0x2C; TxData[2] = getProfileData(0); tryToTransmit(0x040, 3); }
  		  break;
  	  case 1:
  		  if (isSubscribedNv2D) { TxData[1] = 0x2D; TxData[2] = getProfileData(1); tryToTransmit(0x040, 3); }
  		  break;
  	  case 2:
  		  if (isSubscribedNv2E) { TxData[1] = 0x2E; TxData[2] = getProfileData(2); tryToTransmit(0x040, 3); }
  		  break;
  	  case 3:
  		  if (isSubscribedNv2F) { TxData[1] = 0x2F; TxData[2] = getProfileData(3); tryToTransmit(0x040, 3); }
  		  break;
  	  case 4:
  		  if (isSubscribedNv30) { TxData[1] = 0x30; TxData[2] = getProfileData(4); tryToTransmit(0x040, 3); }
  		  break;
  	  case 5:
  		  if (isSubscribedNv31) { TxData[1] = 0x31; TxData[2] = getProfileData(5); tryToTransmit(0x040, 3); }
  		  break;
  	  case 6:
  		  if (isSubscribedNv32) { TxData[1] = 0x32; TxData[2] = getProfileData(6); tryToTransmit(0x040, 3); }
  		  break;
  	  case 7:
  		  if (isSubscribedNv33) { TxData[1] = 0x33; TxData[2] = getProfileData(7); tryToTransmit(0x040, 3); }
  		  break;
  	  case 8:
  		  if (isSubscribedNv34) { TxData[1] = 0x34; TxData[2] = getProfileData(8); tryToTransmit(0x040, 3); }
  		  if (isSubscribedNv35) { TxData[1] = 0x35; TxData[2] = getProfileData(9); tryToTransmit(0x040, 3); }
  		  break;
  	  case 9:
  		  /* not a subscribeable, but a "always present in 200ms cycle" network variables */
  		  TxData[0] = 0xB0;
  		  TxData[1] = 0x92; TxData[2] = 0xFF; TxData[3] = 0xFF; /* unclear NV 92 FF FF */
  		  TxData[4] = 0x93; TxData[5] = 0xE0+profileNumber; TxData[6] = 0x00; /* the "profile" NV 93 E1 00 */
  		  tryToTransmit(0x040, 7);
  		  break;
  }
}

void handle120ms(void) {
	static uint8_t toggleCounter2B4;
	/* The 29C comes each 120ms.
	 * With 20ms offset, the 2BC, 2B4, 3AC, 3A4 come afterwards. */
	if (divider120ms!=DIVIDER_STOPPED) {
		divider120ms++;
		if (divider120ms==20/5) {
			TxData[0] = 0x03; TxData[1] = 0x00;  /* todo: last byte is 0B for 13 rounds. */
			if (pwrM_isShutdownOngoing()) {
				TxData[2] = 0x01; /* shutdown */
			} else {
				if (numberOf2BC<13) {
					TxData[2] = 0x0B; /* at startup, the value 0B is present in 13 messages. */
					numberOf2BC++;
				} else {
					TxData[2] = 0x0F; /* normal run */
				}
			}
			tryToTransmit(0x2BC, 3);
		}
		if (divider120ms==40/5) {
			TxData[0] = 0x03; TxData[1] = 0x00;
			toggleCounter2B4++;
			if ((toggleCounter2B4 % 4)<2) {
				/* In many cases, there are two messages with 02 and two with 04.
				 * But sometimes also only one. */
			    TxData[2] = 0x02;
			} else {
				TxData[2] = 0x0A;
			}
			tryToTransmit(0x2B4, 3);
		}
		if (divider120ms==60/5) {
			TxData[0] = 0x03; TxData[1] = 0x00; TxData[2] = 0x1D;
			tryToTransmit(0x3AC, 3);
		}
		if (divider120ms==80/5) {
			TxData[0] = 0x03; TxData[1] = 0x00;
			if (pwrM_isShutdownOngoing()) {
				TxData[2] = 0x04; /* shutdown */
			} else {
				TxData[2] = 0x2C; /* normal run */
			}
			tryToTransmit(0x3A4, 3);
		}
		if (divider120ms>=120/5) {
			divider120ms=0;
			TxData[0] = 0x03; TxData[1] = 0x00;
			if (pwrM_isShutdownOngoing()) {
				TxData[2] = 0x0A; /* shutdown */
			} else {
				TxData[2] = 0x1A; /* normal run */
			}
			tryToTransmit(0x29C, 3);
		}
	}
}


void runUcmStatemachine(void) {
  switch (ucmOwnState) {
  	  case 0x10: /* The initial state of the UCM is 0x10. The other modules come to the same
  	                state (from 02) at t= ~400ms after the first CAN message.
  	                At t=1.9s, the UCM changes to 0x20. */
  		  if ((powermoduleState==0x10) && (servoLightState==0x10) && (get_tAfterFirstTxMessage_ms()>1900)) {
  			  ucmOwnState = 0x20;
  		  }
  		  break;
  	  case 0x20: /* When the UCM announced the state 0x20, the other modules following with their
  	                state to the same value, within 20ms. We keep sitting here, until the user
  	                wants to drive. */
  		  if ((powermoduleState==0x20) && (servoLightState==0x20) && get_userWantsToDrive()) {
  			  ucmOwnState = 0x23; /* we want to drive */
  		  }
  		  if (pwrM_isShutdownOngoing()) { /* if the power-down was initiated, we change to state 01.
  		                              The other modules will follow this within 20ms. */
  			ucmOwnState = 0x01;
  		  }
  		  break;
  	  case 0x23: /* we want to drive */
		  /* The motor is the first who changes to 0x25. The ServoLightModule follows.
		   * If both reached the 25, this is the trigger for the UCM
		   * to change from 23 to 24 for one message. */
  		  if ((powermoduleState==0x25) && (servoLightState==0x25)) {
  			  ucmOwnState = 0x24;
  		  }
  		  break;
  	  case 0x24: /* There are two cases for state 24:
  	                 1. Start drive: the 24 is sent exactly once, and afterwards 25.
  	                 2. Stop drive: the 24 is sent 8 times (for 140ms), and afterwards 22. */
  		  if (get_userWantsToDrive()) {
  			  ucmOwnState = 0x25; /* The "start drive" situation. */
  		  } else {
  			  nCounterState24DuringStopping++; /* The "stop drive" situation. */
  			  if (nCounterState24DuringStopping>=8) {
  				ucmOwnState = 0x22;
  				nCounterState24DuringStopping = 0; /* preparation for the next time */
  			  }
  		  }
  		break;
  	  case 0x25: /* This is the state during driving. */
  		  if (!get_userWantsToDrive()) { /* If the joystick is in idle position for 2 seconds */
  			ucmOwnState = 0x24; /* we announce 24, the other module keep sending 25. */
  		  }
  		  break;
  	  case 0x22: /* The UCM requested "stop". The others will send 21, then 22, then 20, within ~500ms. */
  		  if ((powermoduleState==0x20) && (servoLightState==0x20)) {
  			  ucmOwnState = 0x20; /* idle state reached */
  		  }
  		  break;
  }
}

void can_mainfunction5ms(void) {
	canTime5ms++;
	startupStep++;

	if ((canTime5ms%4)==1) {
		TxData[0] = 0xAA;
		TxData[1] = 0x04;
		tryToTransmit(0x0AA, 2);
	} else if ((canTime5ms%4)==2) {
		/* each 20ms: run the ucm state machine */
		runUcmStatemachine();
		if (ucmOwnState>=0x20) {
			TxData[0] = 0xB0;
			TxData[1] = 0x01;
			TxData[2] = ucmOwnState; /* starts with 0x10, then changes to 0x20. */
			TxData[3] = 0x90;
			TxData[4] = ucmJoystickY;
			TxData[5] = ucmJoystickX;
			tryToTransmit(0x040, 6);
		} else {
			TxData[0] = 0xB0;
			TxData[1] = 0x01;
			TxData[2] = ucmOwnState; /* starts with 0x10, then changes to 0x20. */
			tryToTransmit(0x040, 3);
		}
	} else if ((canTime5ms%4)==3) {
		handleTransmissionOfSubscribedNVs();
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
		divider120ms = 120/5; /* "expired", immediately send the message */
	}
	if (startupStep==20+40/5+2+2+2 + 150/5 + 40/5) {
		/* request the status from the ServoLightModule 30,02,00,01,08 */
		TxData[0] = 0x30; TxData[1] = 0x02; TxData[2] = 0x00; TxData[3] = 0x01;
		TxData[4] = 0x08;
		tryToTransmit(0x040, 5);
	}
	if (startupStep>1000/5) {
		if ((canTime5ms%4)==0) {
		  /* Demo for light control */
		  uint8_t lightControl = 0x00; /* 0 is lightsOff */
		  if (blLightOn) lightControl |= 0x11; /* low beam */
		  //if (turni_getRightTurn()) {
			//	  lightControl |= 0x22; /* right turn */
		  //}
		  //if (turni_getLeftTurn()) {
				  //lightControl |=  0x44; /* left turn */
		  //}
		  /* For controlling the low beam lights, the original message is
		     0x040 B0 93 E1 00 14 11
		     This contains two network variables, the nr 93 and the nr 14.
		  */
		  //TxData[0] = 0xB0; TxData[1] = 0x93; TxData[2] = 0xE1; TxData[3] = 0x00;
		  //TxData[4] = 0x14; TxData[5] = lightControl;
		  //tryToTransmit(0x040, 6);
		  /* To test, whether the nr 93 is relevant for the lights, we just omit it,
		    and use the short version which only contains the network variable nr 14:
		    0x040 B0 14 11.
		    Result: Also this works perfectly for controlling the lights. */

		    TxData[0] = 0xB0; TxData[1] = 0x14; TxData[2] = lightControl;
		    tryToTransmit(0x040, 3);

		}
	}

	handle120ms();
}

void can_init(void) {
	TxHeader.IDE = CAN_ID_STD; /* standard ID (not extended ID) */
	TxHeader.RTR = CAN_RTR_DATA; /* data message (not remote request message) */
}
