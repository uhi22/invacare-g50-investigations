
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "analogInputs.h"
#include "powerManager.h"
#include "scheduler.h"
#include "buttons.h"
#include "ucm.h"
#include "slm.h"
#include "can_lowlayer.h"
#include "can_application.h"


#define X_COLUMN2 180
static char BufferText1[40];


void showpage1init(void) {
  #define LINESIZEY 20
  ILI9341_FillScreen(BLACK);
  ILI9341_DrawText("counter5ms", FONT3, 10, 0*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("rxCount", FONT3, 10, 1*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("buttonfield", FONT3, 10, 2*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("adc0", FONT3, 10, 3*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("adc1", FONT3, 10, 4*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("adc2", FONT3, 10, 5*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("adc3", FONT3, 10, 6*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("....", FONT3, 10, 7*LINESIZEY, GREENYELLOW, BLACK);

  ILI9341_DrawText("....", FONT3, 10, 8*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText(".....", FONT3, 10, 9*LINESIZEY, GREENYELLOW, BLACK);

  /* second column */
  ILI9341_DrawText("ucmOwnState", FONT3, X_COLUMN2, 0*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("servoLightState", FONT3, X_COLUMN2, 1*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("motorState", FONT3, X_COLUMN2, 2*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("ucmError", FONT3, X_COLUMN2, 3*LINESIZEY, GREENYELLOW, BLACK);
  
}

void showpage1cyclic(void) {
	switch (displaySubTick) { /* only refresh the page partly each call, to avoid delaying the CAN message stuff. */
		case 0:
			sprintf(BufferText1, "%ld  ", counter5ms);
			(void)display_drawString(BufferText1, 100, 0*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%ld %ld ", nNumberOfReceivedMessages, nNumberOfCanInterrupts);
			(void)display_drawString(BufferText1, 100, 1*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%04x ", buttonField);
			(void)display_drawString(BufferText1, 100, 2*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 1:
			sprintf(BufferText1, "%4d  ", adcValues[0]);
			(void)display_drawString(BufferText1, 100, 3*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%4d  ", adcValues[1]);
			(void)display_drawString(BufferText1, 100, 4*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%4d  ", adcValues[2]);
			(void)display_drawString(BufferText1, 100, 5*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%4d  ", adcValues[3]);
			(void)display_drawString(BufferText1, 100, 6*LINESIZEY, GREENYELLOW, BLACK, 2);

			break;
		case 2:
			sprintf(BufferText1, "%2d  ", ucmOwnState);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 0*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%2d  ", servoLightState);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 1*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%2d  ", powermoduleState);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 2*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%3d  ", ucmError);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 3*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 3:
			//sprintf(BufferText1, "%d  ", ucmJoystickY);
			//(void)display_drawString(BufferText1, 100, 6*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%2d  ", canTxQueueUsedSize);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 5*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%2d  ", canTxQueueUsedSizeMax);
			(void)display_drawString(BufferText1, X_COLUMN2 + 100, 6*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 4:
			//sprintf(BufferText1, "%d  ", motorUBattRaw);
			//(void)display_drawString(BufferText1, 100, 7*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "%d  ", ucmLightDemand);
			//(void)display_drawString(BufferText1, 100, 8*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 5:
			//sprintf(BufferText1, "%ld  ", canTxErrorCounter);
			//(void)display_drawString(BufferText1, 100, 9*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "%ld  ", canTxOkCounter);
			//(void)display_drawString(BufferText1, 100, 10*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "adc %d %d  ", adcValues[0], adcValues[1]);
			//(void)display_drawString(BufferText1, 200, 10*LINESIZEY, GREENYELLOW, BLACK, 2);

			//sprintf(BufferText1, "bf %d ", buttonField);
			//(void)display_drawString(BufferText1, 200, 11*LINESIZEY, GREENYELLOW, BLACK, 2);

			break;
		case 6:
			break;
		case 7:
			if (pwrM_isShutdownOngoing()) {
				sprintf(BufferText1, "OFF %d", pwrM_shutdownTimer);
				(void)display_drawString(BufferText1, 2, 200, GREENYELLOW, BLACK, 2);
			}
			break;
	}
}

#ifdef USE_PAGE_2
void showpage2init(void) {
  #define LINESIZEY 20
  ILI9341_FillScreen(BLACK);
  ILI9341_DrawText("loops", FONT3, 10, 0*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("rxCount", FONT3, 10, 1*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("ucmState", FONT3, 10, 2*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("motState", FONT3, 10, 3*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("srvoLghtSt", FONT3, 10, 4*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("joyst X", FONT3, 10, 5*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("joyst Y", FONT3, 10, 6*LINESIZEY, GREENYELLOW, BLACK);

  ILI9341_DrawText("motUBatRw", FONT3, 10, 7*LINESIZEY, GREENYELLOW, BLACK);
  ILI9341_DrawText("lightDemd", FONT3, 10, 8*LINESIZEY, GREENYELLOW, BLACK);

  /* second column */
  ILI9341_DrawText("servoPos", FONT3, X_COLUMN2, 0*LINESIZEY, GREENYELLOW, BLACK);
}


void showpage2cyclic(void) {
	switch (displaySubTick) { /* only refresh the page partly each call, to avoid delaying the CAN message stuff. */
		case 0:
			//sprintf(BufferText1, "%d  ", nMainLoops);
			//(void)display_drawString(BufferText1, 100, 0*LINESIZEY, GREENYELLOW, BLACK, 2);
			//(void)display_drawString(BufferText, 150, 130, GREENYELLOW, DARKCYAN, 6);
			//(void)display_drawString(BufferText, 150, 190, YELLOW, BLUE, 7);

			//sprintf(BufferText1, "%ld  ", nNumberOfReceivedMessages);
			//(void)display_drawString(BufferText1, 100, 1*LINESIZEY, GREENYELLOW, BLACK, 2);

			//sprintf(BufferText1, "%i ", servoPosition);
			//(void)display_drawString(BufferText1, X_COLUMN2+80, 0*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 1:
			//sprintf(BufferText1, "%d  ", ucmOwnState);
			//(void)display_drawString(BufferText1, 100, 2*LINESIZEY, GREENYELLOW, BLACK, 2);

			//sprintf(BufferText1, "%d  ", motorState);
			//(void)display_drawString(BufferText1, 100, 3*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 2:
			//sprintf(BufferText1, "%d  ", servoLightState);
			//(void)display_drawString(BufferText1, 100, 4*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%d  ", ucmJoystickX);
			(void)display_drawString(BufferText1, 100, 5*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 3:
			sprintf(BufferText1, "%d  ", ucmJoystickY);
			(void)display_drawString(BufferText1, 100, 6*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 4:
			//sprintf(BufferText1, "%d  ", motorUBattRaw);
			//(void)display_drawString(BufferText1, 100, 7*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "%d  ", ucmLightDemand);
			//(void)display_drawString(BufferText1, 100, 8*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 5:
			//sprintf(BufferText1, "%ld  ", canTxErrorCounter);
			//(void)display_drawString(BufferText1, 100, 9*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "%ld  ", canTxOkCounter);
			//(void)display_drawString(BufferText1, 100, 10*LINESIZEY, GREENYELLOW, BLACK, 2);
			//sprintf(BufferText1, "adc %d %d  ", adcValues[0], adcValues[1]);
			//(void)display_drawString(BufferText1, 200, 10*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "bf %d ", buttonField);
			(void)display_drawString(BufferText1, 200, 11*LINESIZEY, GREENYELLOW, BLACK, 2);

			break;
		case 6:
			if ((nNumberOfReceivedMessages & 0x08)) {
			  ILI9341_DrawRectangle(316, 0, 2, 2, GREENYELLOW);
			} else {
			  ILI9341_DrawRectangle(316, 0, 2, 2, BLACK);
			}
			break;
		case 7:
			if ((nNumberOfReceivedMessages & 0x04)) {
			  ILI9341_DrawRectangle(316, 3, 2, 2, GREENYELLOW);
			} else {
			  ILI9341_DrawRectangle(316, 3, 2, 2, BLACK);
			}
			if (pwrM_isShutdownOngoing()) {
				sprintf(BufferText1, "OFF %d", pwrM_shutdownTimer);
				(void)display_drawString(BufferText1, 2, 200, GREENYELLOW, BLACK, 2);
			}
			break;
	}
}
#endif
