
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "Font7s.h"
#include "Font16.h"
#include "Font32.h"
//#include "Font64.h"
#include "canbus.h"
#include <stdio.h>
#include <string.h>
#include "flashhandler.h"
#include "buttons.h"
#include "powerManager.h"
#include "ucm.h"
#include "powermodule.h"

/* fixed-size-font: e.g. this: https://github.com/idispatch/raster-fonts/blob/master/font-9x16.c */

#define X_COLUMN2 180

extern uint32_t nNumberOfReceivedMessages;
extern uint32_t nNumberOfCanInterrupts;
extern uint32_t canTxErrorCounter, canTxOkCounter;
extern uint16_t adcValues[4];
extern uint8_t lightControl;


uint8_t nShowPageFraction;

#define COLOR_BUFFER_SIZE 6000 /* bytes for one character. Is twice the pixel count of one character. */
uint8_t myColorBuffer[COLOR_BUFFER_SIZE];
uint16_t colorBufferIndex;
static char BufferText1[40];

uint8_t nCurrentPage, nLastPage;
uint16_t counterPageSwitch;
extern uint16_t nMainLoops;



extern unsigned char console_font_12x16[];

uint16_t drawChar12x16(char ch, uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor) {
    uint16_t width;
    uint16_t height;
    uint16_t pixelColor;
    uint8_t const *charBitmapPtr;
    uint16_t bitnr;
    uint8_t mask;
    uint8_t bytesPerLine;
    uint8_t yFactor = 6;
    uint8_t xFactor = 3;
    uint8_t n,m;

    width = 12;
    height = 16;

    charBitmapPtr = &console_font_12x16[2*height*(uint16_t)ch];

    int i;
    colorBufferIndex = 0;
    bytesPerLine = (width+7)/8;
	for (int j=0; j < height; j++)
	{
        bitnr = 0;
		for (i=0; i < width; i++) {
            mask = 1 << (7 - (bitnr%8));
            if (charBitmapPtr[j*bytesPerLine + bitnr/8] & mask) {
                pixelColor = color;
             } else {
                pixelColor = bgcolor;
            }
            //ILI9341_DrawPixel(X+i, Y+j, pixelColor);
            myColorBuffer[colorBufferIndex] = (uint8_t)(pixelColor >> 8);
            myColorBuffer[colorBufferIndex+1] = (uint8_t)pixelColor;
            if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
              colorBufferIndex+=2;
            }
            for (m=1; m<xFactor; m++) {
				myColorBuffer[colorBufferIndex] = (uint8_t)(pixelColor >> 8);
				myColorBuffer[colorBufferIndex+1] = (uint8_t)pixelColor;
				if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
				  colorBufferIndex+=2;
				}
            }
            bitnr++;
		}
		for (n=1; n<yFactor; n++) {
			for (i=0; i < xFactor*width; i++) {
				uint16_t lineOffset = xFactor*2*width; /* pixel per line */
				myColorBuffer[colorBufferIndex]   = myColorBuffer[colorBufferIndex-lineOffset];
				myColorBuffer[colorBufferIndex+1] = myColorBuffer[colorBufferIndex+1-lineOffset];
				if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
				  colorBufferIndex+=2;
				}
			}
		}
	}
	ILI9341_SetAddress(X, Y, X+xFactor*width-1, Y+yFactor*height-1);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(HSPI_INSTANCE, myColorBuffer, colorBufferIndex, 10);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
    return xFactor*width;
}

uint16_t TestGraphics_DrawChar(char ch, uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    uint16_t width;
    uint16_t height;
    uint16_t pixelColor;
    uint8_t const *charBitmapPtr;
    int16_t gap;
    uint16_t bitnr;
    uint8_t mask;
    uint8_t bytesPerLine;
    uint8_t isDoubleWidth = 0;
    uint8_t isDoubleHeight = 0;
    int i;

    if (size & 64) {
    	isDoubleHeight = 1;
    	size &= ~64;
    }

	if ((ch < 32) || (ch > 127)) return 0;
    ch = ch - 32;
    if (size == 2) {
      charBitmapPtr = chrtbl_f16[(uint8_t)ch];
      width = widtbl_f16[(uint8_t)ch];
      height = chr_hgt_f16;
      gap = 1;
    }
    if (size == 4) {
      charBitmapPtr = chrtbl_f32[(uint8_t)ch];
      width = widtbl_f32[(uint8_t)ch];
      height = chr_hgt_f32;
      gap = -3;
    }
   if (size == 6) {
      //charBitmapPtr = chrtbl_f64[(uint8_t)ch];
      //width = widtbl_f64[(uint8_t)ch];
      //height = chr_hgt_f64;
      //gap = -3;
   }
    if (size == 7) {
      charBitmapPtr = chrtbl_f7s[(uint8_t)ch];
      width = widtbl_f7s[(uint8_t)ch];
      height = chr_hgt_f7s;
      gap = 2;
    }
    //if (size == 9) {
    //	return drawChar12x16(ch, X, Y, color, bgcolor);
    //}
    colorBufferIndex = 0;
    bytesPerLine = (width+7)/8;
	for (int j=0; j < height; j++)
	{
        bitnr = 0;
		for (i=0; i < width; i++)
		{
            mask = 1 << (7 - (bitnr%8));
            if (charBitmapPtr[j*bytesPerLine + bitnr/8] & mask) {
                pixelColor = color;
             } else {
                pixelColor = bgcolor;
            }
            //ILI9341_DrawPixel(X+i, Y+j, pixelColor);
            myColorBuffer[colorBufferIndex] = (uint8_t)(pixelColor >> 8);
            myColorBuffer[colorBufferIndex+1] = (uint8_t)pixelColor;
            if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
              colorBufferIndex+=2;
            }
            if (isDoubleWidth) {
                myColorBuffer[colorBufferIndex] = (uint8_t)(pixelColor >> 8);
                myColorBuffer[colorBufferIndex+1] = (uint8_t)pixelColor;
                if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
                  colorBufferIndex+=2;
                }
            }
            bitnr++;
		}
		if (isDoubleHeight) {
			/* we duplicate the complete last line */
			for (i=0; i < width; i++) {
			  uint16_t lineOffset = 2*width; /* pixel per line */
              myColorBuffer[colorBufferIndex]   = myColorBuffer[colorBufferIndex-lineOffset];
              myColorBuffer[colorBufferIndex+1] = myColorBuffer[colorBufferIndex+1-lineOffset];
              if (colorBufferIndex<COLOR_BUFFER_SIZE-2) {
                colorBufferIndex+=2;
              }
			}
		}
	}
	if (!isDoubleWidth) {
		if (isDoubleHeight) {
			ILI9341_SetAddress(X, Y, X+width-1, Y+2*height-1);
			//ILI9341_DrawColorBurst(color, height*width);
			HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
			HAL_SPI_Transmit(HSPI_INSTANCE, myColorBuffer, colorBufferIndex, 10);
			HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
			return width+gap;
		} else {
			ILI9341_SetAddress(X, Y, X+width-1, Y+height-1);
			//ILI9341_DrawColorBurst(color, height*width);
			HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
			HAL_SPI_Transmit(HSPI_INSTANCE, myColorBuffer, colorBufferIndex, 10);
			HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
			return width+gap;
		}
	} else {
		/* twice the size */
		ILI9341_SetAddress(X, Y, X+2*width-1, Y+height-1);
		//ILI9341_DrawColorBurst(color, height*width);
		HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
		HAL_SPI_Transmit(HSPI_INSTANCE, myColorBuffer, colorBufferIndex, 10);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
		return 2*width+gap;
	}
}

int16_t TestGraphics_drawString(char *string, int16_t poX, int16_t poY, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    int16_t sumX = 0;

    while(*string)
    {
        int16_t xPlus = TestGraphics_DrawChar(*string, poX, poY, color, bgcolor, size);
        sumX += xPlus;
        string++;
        poX += xPlus;                            /* Move cursor right       */
    }
    return sumX;
}


void showpage1(uint8_t blInit) {
    #define LINESIZEY 20
	if (blInit) {
		ILI9341_FillScreen(BLACK);
		  ILI9341_DrawText("loops", FONT3, 10, 0*LINESIZEY, GREENYELLOW, BLACK);
		  ILI9341_DrawText("rxCount", FONT3, 10, 1*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("rxUpTime", FONT3, 10, 2*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("checkpoint", FONT3, 10, 3*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("debug", FONT3, 10, 4*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("EVSEPresentV", FONT2, 0, 170, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("uCcsInlet_V", FONT3, 200, 170, GREENYELLOW, BLACK);

		  //ILI9341_DrawText("Temperatures [celsius]", FONT1, 180, 0*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("T1", FONT3, 180, 1*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("T2", FONT3, 180, 2*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("T3", FONT3, 180, 3*LINESIZEY, GREENYELLOW, BLACK);
		  //ILI9341_DrawText("CPU", FONT3, 180, 4*LINESIZEY, GREENYELLOW, BLACK);
		  ILI9341_DrawHollowRectangleCoord(179, 0, 309, 5*LINESIZEY, DARKCYAN);
	}
    sprintf(BufferText1, "%d  ", nMainLoops);
    (void)TestGraphics_drawString(BufferText1, 100, 0*LINESIZEY, GREENYELLOW, BLACK, 4);
    //(void)TestGraphics_drawString(BufferText, 150, 130, GREENYELLOW, DARKCYAN, 6);
    //(void)TestGraphics_drawString(BufferText, 150, 190, YELLOW, BLUE, 7);

    sprintf(BufferText1, "%ld  ", nNumberOfReceivedMessages);
    (void)TestGraphics_drawString(BufferText1, 100, 1*LINESIZEY, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%ld  ", canRxDataUptime);
    //(void)TestGraphics_drawString(BufferText1, 100, 2*LINESIZEY, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%d  ", canRxCheckpoint);
    //(void)TestGraphics_drawString(BufferText1, 100, 3*LINESIZEY, GREENYELLOW, BLACK, 4);

    //sprintf(BufferText1, "%d  ", canDebugValue1);
    //(void)TestGraphics_drawString(BufferText1, 100, 4*LINESIZEY, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%d  ", canDebugValue2);
    //(void)TestGraphics_drawString(BufferText1, 100, 4*LINESIZEY+1*16, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%d  ", canDebugValue3);
    //(void)TestGraphics_drawString(BufferText1, 100, 4*LINESIZEY+2*16, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%d  ", canDebugValue4);
    //(void)TestGraphics_drawString(BufferText1, 100, 4*LINESIZEY+3*16, GREENYELLOW, BLACK, 2);

    //sprintf(BufferText1, "%d  ", EVSEPresentVoltage);
    //(void)TestGraphics_drawString(BufferText1, 0, 182, GREENYELLOW, BLACK, 6);

    //sprintf(BufferText1, "%d  ", uCcsInlet_V);
    //(void)TestGraphics_drawString(BufferText1, 200, 182, GREENYELLOW, BLACK, 6);


    if ((nNumberOfReceivedMessages & 0x08)) {
  	  ILI9341_DrawRectangle(310, 0, 5, 5, GREENYELLOW);
    } else {
  	  ILI9341_DrawRectangle(310, 0, 5, 5, BLACK);
    }
    if ((nNumberOfReceivedMessages & 0x04)) {
  	  ILI9341_DrawRectangle(310, 9, 5, 5, GREENYELLOW);
    } else {
  	  ILI9341_DrawRectangle(310, 9, 5, 5, BLACK);
    }
    if ((nNumberOfReceivedMessages & 0x02)) {
  	  ILI9341_DrawRectangle(310, 18, 5, 5, GREENYELLOW);
    } else {
  	  ILI9341_DrawRectangle(310, 18, 5, 5, BLACK);
    }

}


#define RGB_TO_TFT(r, g, b) (((r / 8) << 11) | ((g / 4) << 5) | (b / 8))

#define BACKGROUNDCOLOR RGB_TO_TFT(90, 90, 90)
#define BROWN RGB_TO_TFT(150, 60, 0)
#define MY_ORANGE RGB_TO_TFT(255, 170, 50)
#define TICK_COLOR TFT_WHITESMOKE

uint16_t getColorFromTable(uint8_t x) {
	if (x<10) return BLACK;
	if (x<20) return BROWN;
	if (x<30) return RED;
	if (x<40) return MY_ORANGE;
	if (x<50) return YELLOW;
	if (x<60) return GREEN;
	if (x<70) return BLUE;
	if (x<80) return PINK;
	if (x<90) return LIGHTGREY;
	return WHITE;
}





/* The DXBUS default page */
void showpage3(uint8_t blInit) {
    #define LINESIZEY 20
	if (blInit) {
		ILI9341_FillScreen(BLACK);
		ILI9341_DrawText("loops", FONT3, 10, 0*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("rxCount", FONT3, 10, 1*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("ucmState", FONT3, 10, 2*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("pmState", FONT3, 10, 3*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("srvoLghtSt", FONT3, 10, 4*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("joyst X", FONT3, 10, 5*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("joyst Y", FONT3, 10, 6*LINESIZEY, GREENYELLOW, BLACK);

		ILI9341_DrawText("motUBatRw", FONT3, 10, 7*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("lightDemd", FONT3, 10, 8*LINESIZEY, GREENYELLOW, BLACK);

		/* second column */
		ILI9341_DrawText("servoPos", FONT3, X_COLUMN2, 0*LINESIZEY, GREENYELLOW, BLACK);
		ILI9341_DrawText("lightcontrol", FONT3, X_COLUMN2, 1*LINESIZEY, GREENYELLOW, BLACK);

	}

    //sprintf(BufferText1, "%d ", ucmState);
    //(void)TestGraphics_drawString(BufferText1, 10, 165, GREENYELLOW, BLACK, 7);
	nShowPageFraction++;
	switch (nShowPageFraction%8) { /* only refresh the page partly each call, to avoid delaying the CAN message stuff. */
		case 0:
			sprintf(BufferText1, "%d  ", nMainLoops);
			(void)TestGraphics_drawString(BufferText1, 100, 0*LINESIZEY, GREENYELLOW, BLACK, 2);
			//(void)TestGraphics_drawString(BufferText, 150, 130, GREENYELLOW, DARKCYAN, 6);
			//(void)TestGraphics_drawString(BufferText, 150, 190, YELLOW, BLUE, 7);

			sprintf(BufferText1, "%ld  ", nNumberOfReceivedMessages);
			(void)TestGraphics_drawString(BufferText1, 100, 1*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%i ", servoPosition);
			(void)TestGraphics_drawString(BufferText1, X_COLUMN2+80, 0*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%02x ", lightControl);
			(void)TestGraphics_drawString(BufferText1, X_COLUMN2+80, 1*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 1:
			sprintf(BufferText1, "%d  ", ucmOwnState);
			(void)TestGraphics_drawString(BufferText1, 100, 2*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%d  ", powermoduleState);
			(void)TestGraphics_drawString(BufferText1, 100, 3*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 2:
			sprintf(BufferText1, "%d  ", servoLightState);
			(void)TestGraphics_drawString(BufferText1, 100, 4*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "%d  ", ucmJoystickX);
			(void)TestGraphics_drawString(BufferText1, 100, 5*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 3:
			sprintf(BufferText1, "%d  ", ucmJoystickY);
			(void)TestGraphics_drawString(BufferText1, 100, 6*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 4:
			sprintf(BufferText1, "%d  ", motorUBattRaw);
			(void)TestGraphics_drawString(BufferText1, 100, 7*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%d  ", ucmLightDemand);
			(void)TestGraphics_drawString(BufferText1, 100, 8*LINESIZEY, GREENYELLOW, BLACK, 2);
			break;
		case 5:
			sprintf(BufferText1, "%ld  ", canTxErrorCounter);
			(void)TestGraphics_drawString(BufferText1, 100, 9*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "%ld  ", canTxOkCounter);
			(void)TestGraphics_drawString(BufferText1, 100, 10*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "adc %d %d  ", adcValues[0], adcValues[1]);
			(void)TestGraphics_drawString(BufferText1, 200, 10*LINESIZEY, GREENYELLOW, BLACK, 2);

			sprintf(BufferText1, "bf %d ", buttonField);
			(void)TestGraphics_drawString(BufferText1, 200, 11*LINESIZEY, GREENYELLOW, BLACK, 2);

			break;
		case 6:
			sprintf(BufferText1, "suc %d ", canTxQueueSuccessfulQueuedCounter);
			(void)TestGraphics_drawString(BufferText1, 200, 3*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "ovr %d ", canTxQueueOverruns);
			(void)TestGraphics_drawString(BufferText1, 200, 4*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "mbfr %d ", can_txMailboxFreeCounter);
			(void)TestGraphics_drawString(BufferText1, 200, 5*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "txci %d ", nNumberOfCanTxCompleteInterrupts);
			(void)TestGraphics_drawString(BufferText1, 200, 6*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "ci %d ", debug_CAN_IRQ_Counter);
			(void)TestGraphics_drawString(BufferText1, 200, 7*LINESIZEY, GREENYELLOW, BLACK, 2);
			sprintf(BufferText1, "itsr %lx ", debug_CAN_tsrflags);
			(void)TestGraphics_drawString(BufferText1, 200, 8*LINESIZEY, GREENYELLOW, BLACK, 2);




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
				(void)TestGraphics_drawString(BufferText1, 2, 200, GREENYELLOW, BLACK, 2);
			}
			break;
	}


}


void display_update20ms(void) {
	uint32_t uptime_s;
	uptime_s = HAL_GetTick() / 1000; /* the uptime in seconds */
	if (uptime_s>1) {
			nCurrentPage=3;
	}
	if (nLastPage!=nCurrentPage) {
		/* page changed. Clear and prepare the static content. */
		if (nCurrentPage==1) showpage1(1);
		//if (nCurrentPage==2) showpage2(1);
		if (nCurrentPage==3) showpage3(1);
		nLastPage = nCurrentPage;
	}
	if (nCurrentPage==3) showpage3(0);

}


