
#include "stm32f1xx_hal.h"

extern void setKeepPower(uint8_t on);
extern void setBusWake(uint8_t on);
extern void setLED_D1(uint8_t on);
extern void setLED_D11(uint8_t on);
extern void setLEDAlive(uint8_t on);
extern void setOut1(uint8_t on);
extern void setOut2(uint8_t on);
extern void setOut3(uint8_t on);


extern uint8_t getSwitch1(void);
extern uint8_t getSwitch2(void);
extern uint8_t getSwitch3(void);
extern uint8_t getSwitch4(void);
extern uint8_t getButton1(void);
extern uint8_t getButton2(void);
extern uint8_t getButton3(void);
extern uint8_t getButton4(void);
extern uint8_t getJoystickButton(void);
extern uint8_t getPowerButton(void);
