
#include "hardwareAbstraction.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"


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


void display_update20ms(void) {

}

void display_init(void) {
  uint16_t i;
  ILI9341_Init();
  ILI9341_FillScreen(BLACK);
  ILI9341_SetRotation(SCREEN_HORIZONTAL_2);
  ILI9341_DrawText("HELLO WORLD", FONT4, 90, 80, WHITE, BLACK);
  ILI9341_DrawText("github.com/uhi22/", FONT4, 100, 110, WHITE, BLACK);
  ILI9341_DrawText("invacare-g50-investigations", FONT4, 80, 130, WHITE, BLACK);
  for(i = 50; i <= 300; i+=3) {
	  ILI9341_DrawVLine(i, 150, 30, DARKGREEN);
	  HAL_Delay(8);
  }
  HAL_Delay(100);
}

