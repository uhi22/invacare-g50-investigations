
#include "hardwareAbstraction.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "Font7s.h"
#include "Font16.h"
#include "Font32.h"

extern void display_init(void);
extern void display_mainfunction20ms(void);

extern void showpage1init(void);
extern void showpage1cyclic(void);

extern int16_t display_drawString(char *string, int16_t poX, int16_t poY, uint16_t color, uint16_t bgcolor, uint8_t size);

extern uint8_t displaySubTick;
