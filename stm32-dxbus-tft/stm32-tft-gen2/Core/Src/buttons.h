

extern uint8_t buttonField;

extern void buttons_mainfunction(void);

#define BUTTON_MASK_POWER 1
#define BUTTON_MASK_LEFT 2
#define BUTTON_MASK_MIDDLE 4
#define BUTTON_MASK_RIGHT 8
#define BUTTON_MASK_JOYSTICK 0x10

#define BUTTON_MASK_LIGHT BUTTON_MASK_MIDDLE
