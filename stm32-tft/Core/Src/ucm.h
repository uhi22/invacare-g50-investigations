
#define ADC_CHANNEL_JOYSTICK_Y 0
#define ADC_CHANNEL_JOYSTICK_X 1

extern uint16_t adcValues[4];
extern uint8_t ucmJoystickX, ucmJoystickY;
extern uint8_t blLightOn;
extern uint8_t flasherMode;

extern void runJoystickMain5ms(void);
extern uint8_t get_userWantsToDrive(void);

