


extern uint8_t ucmOwnState;
extern uint8_t ucmJoystickX, ucmJoystickY;
extern uint8_t blLightOn;
extern uint8_t flasherMode;


extern uint8_t ucmError;
#define UCM_ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_X 91
#define UCM_ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_Y 92
#define UCM_ERR_NOT_NEUTRAL_DURING_STARTUP_POTI 93

extern void ucm_mainfunction5ms(void);
extern uint8_t get_userWantsToDrive(void);

