
#include "display.h"

uint8_t nCurrentPage, nLastPage;
uint8_t displaySubTick;

#define COLOR_BUFFER_SIZE 6000 /* bytes for one character. Is twice the pixel count of one character. */
uint8_t myColorBuffer[COLOR_BUFFER_SIZE];
uint16_t colorBufferIndex;

uint16_t display_DrawChar(char ch, uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint8_t size)
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

int16_t display_drawString(char *string, int16_t poX, int16_t poY, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    int16_t sumX = 0;

    while(*string)
    {
        int16_t xPlus = display_DrawChar(*string, poX, poY, color, bgcolor, size);
        sumX += xPlus;
        string++;
        poX += xPlus;                            /* Move cursor right       */
    }
    return sumX;
}

void display_demo(void) {
  /*
  ILI9341_FillScreen(WHITE);

  for(i = 0; i <= 5; i++)
  {
    sprintf(BufferText, "COUNT : %d", i);
    ILI9341_DrawText(BufferText, FONT3, 10, 10, BLACK, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 30, BLUE, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 50, RED, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 70, GREEN, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 90, YELLOW, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 110, PURPLE, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 130, ORANGE, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 150, MAROON, WHITE);
    ILI9341_DrawText(BufferText, FONT3, 10, 170, WHITE, BLACK);
    ILI9341_DrawText(BufferText, FONT3, 10, 190, BLUE, BLACK);
  }
 */
  // Horizontal Line (X, Y, Length, Color)
  //ILI9341_FillScreen(WHITE);
  //ILI9341_DrawHLine(50, 120, 200, NAVY);
  //HAL_Delay(1000);

  // Vertical Line (X, Y, Length, Color)
  //ILI9341_FillScreen(WHITE);
  //ILI9341_DrawVLine(160, 40, 150, DARKGREEN);
  //HAL_Delay(1000);

  // Hollow Circle (Centre X, Centre Y, Radius, Color)
  //ILI9341_FillScreen(WHITE);
  //ILI9341_DrawHollowCircle(160, 120, 80, PINK);
  //HAL_Delay(1000);

  // Filled Rectangle (Start X, Start Y, Length X, Length Y)
  //ILI9341_FillScreen(WHITE);
  //ILI9341_DrawRectangle(50, 50, 220, 140, GREENYELLOW);
  //HAL_Delay(1000);

  // Hollow Rectangle (Start X, Start Y, End X, End Y)
  //ILI9341_FillScreen(WHITE);
  //ILI9341_DrawHollowRectangleCoord(50, 50, 270, 190, DARKCYAN);
  //HAL_Delay(100);
}


void display_mainfunction20ms(void) {
	uint32_t uptime_s;
	uptime_s = HAL_GetTick() / 1000; /* the uptime in seconds */
	if (uptime_s>1) {
			nCurrentPage=1;
	}
	displaySubTick++;
	displaySubTick &= 0x07; /* subTick in range 0 to 7, to create 160ms cycle. */
	if (nLastPage!=nCurrentPage) {
		/* page changed. Clear and prepare the static content. */
		//if (nCurrentPage==1) showpage1init();
		nLastPage = nCurrentPage;
	}
	if (nCurrentPage==1) showpage1cyclic();
}

void display_init(void) {
  uint16_t i;
  ILI9341_Init();
  ILI9341_FillScreen(BLACK);
  ILI9341_SetRotation(SCREEN_HORIZONTAL_2);
  (void)display_drawString("DX DASH", 110, 40, GREENYELLOW, BLACK, 4+64);
  ILI9341_DrawText("Generation 2", FONT4, 110, 90, WHITE, BLACK);
  ILI9341_DrawText("Version 2025-01-03", FONT4, 85, 120, WHITE, BLACK);

  ILI9341_DrawText("github.com/uhi22/invacare-g50-investigations", FONT2, 30, 150, WHITE, BLACK);
  for(i = 40; i <= 280; i+=3) {
	  ILI9341_DrawVLine(i, 190, 30, GREEN);
	  HAL_Delay(20);
  }
  HAL_Delay(100);
  nCurrentPage=0;
  nLastPage=0;
  showpage1init();
}

