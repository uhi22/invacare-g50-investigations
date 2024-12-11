
#include "stm32f1xx_hal.h"

extern void ADC_cyclicScan(void);

#define NUMBER_OF_ADC_CHANNELS 4

extern uint16_t adcValues[NUMBER_OF_ADC_CHANNELS];

