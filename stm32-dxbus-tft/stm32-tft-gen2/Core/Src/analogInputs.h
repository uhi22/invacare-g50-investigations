
#include "stm32f1xx_hal.h"

extern void ADC_cyclicScan(void);

#define ADC_CHANNEL_POTI1      0
#define ADC_CHANNEL_POTI2      1
#define ADC_CHANNEL_JOYSTICK_Y 2
#define ADC_CHANNEL_JOYSTICK_X 3
#define NUMBER_OF_ADC_CHANNELS 4

extern uint16_t adcValues[NUMBER_OF_ADC_CHANNELS];

