
#include "analogInputs.h"

/*

ADC without DMA is explained here: https://controllerstech.com/stm32-adc-multi-channel-without-dma/

*/
extern void Error_Handler(void);
extern ADC_HandleTypeDef hadc1;
uint16_t adcValues[NUMBER_OF_ADC_CHANNELS];


void ADC_Select_CH0 (void) {
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
	  sConfig.Channel = ADC_CHANNEL_10; /* PC0, poti1 */
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

void ADC_Select_CH1 (void) {
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
	  sConfig.Channel = ADC_CHANNEL_11; /* PC1, poti2 */
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

void ADC_Select_CH2 (void) {
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
	  sConfig.Channel = ADC_CHANNEL_12; /* PC2, joystickY */
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

void ADC_Select_CH3 (void) {
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
	  sConfig.Channel = ADC_CHANNEL_13; /* PC3, joystickX */
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

void ADC_cyclicScan(void) {
    /* todo: Measure how long this blocks, and make it unblocking if necessaery */
	  ADC_Select_CH0();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[ADC_CHANNEL_POTI1] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  ADC_Select_CH1();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[ADC_CHANNEL_POTI2] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  ADC_Select_CH2();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[ADC_CHANNEL_JOYSTICK_Y] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  ADC_Select_CH3();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[ADC_CHANNEL_JOYSTICK_X] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);


}
