extern uint8_t globalError;

#define ERR_OK 0
#define ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_X 1
#define ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_Y 2
#define ERR_PEDAL_NOT_NEUTRAL_DURING_STARTUP 3
#define ERR_NO_RESPONSE_FROM_POWER_UNIT 4
#define ERR_UNDERVOLTAGE 5
#define ERR_DRIVEPEDAL_SIGNAL_TOO_LOW 6
#define ERR_DRIVEPEDAL_SIGNAL_TOO_HIGH 7
#define ERR_POWERMODULE_UNCOUPLED_BLINK3 8
#define ERR_POWERMODULE_M1_OPENLOAD_BLINK5 9
#define ERR_POWERMODULE_UNDERVOLTAGE_BLINK7 10
#define ERR_POWERMODULE_OTHER 11

#define ERROR_TEXT_TABLE { \
		/*  0 */ "OK", \
		/*  1 */ "Joystick X nicht neutral beim Starten",\
		/*  2 */ "Joystick Y nicht neutral beim Starten",\
		/*  3 */ "Fahrpedal nicht neutral beim Starten",\
		/*  4 */ "Keine Antwort vom Powermodul",\
		/*  5 */ "Spannung zu niedrig",\
		/*  6 */ "Fahrpedal Signal zu niedrig",\
		/*  7 */ "Fahrpedal Signal zu hoch",\
		/*  8 */ "Powermodul ausgekuppelt, Blinkcode 3",\
		/*  9 */ "Powermodul M1 getrennt, Blinkcode 5",\
		/* 10 */ "Powermodul Unterspannung<17V, Blinkcode 7",\
		/* 11 */ "Powermodul weiterer Fehler",\
}


#define isJoystickError() (     (globalError==ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_X) || \
                                (globalError==ERR_NOT_NEUTRAL_DURING_STARTUP_JOYSTICK_Y))

#define isDrivePedalError() (   (globalError==ERR_PEDAL_NOT_NEUTRAL_DURING_STARTUP) || \
                                (globalError==ERR_DRIVEPEDAL_SIGNAL_TOO_LOW) || \
                                (globalError==ERR_DRIVEPEDAL_SIGNAL_TOO_HIGH))
                                