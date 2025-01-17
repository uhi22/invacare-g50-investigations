/* USER CODE BEGIN Header */
/**

  TFT Display with CAN

  STM32F103C8T6 "BluePill"
  TFT with ILI9341 via SPI
  CAN

  ADC without DMA is explained here: https://controllerstech.com/stm32-adc-multi-channel-without-dma/


  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "canbus.h"
#include <stdio.h>
#include "testGraphics.h"
#include "flashhandler.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "scheduler.h"
#include "hardwareinterface.h"
#include "powermodule.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef   TxHeader;
uint8_t               TxData[8];
uint32_t              TxMailbox;
CAN_RxHeaderTypeDef canRxMsgHdr;
uint8_t canRxData[8];
uint16_t adcValues[4];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_CAN_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void canbus_demoTransmit(void) {
  long int rc;
  uint32_t uptime_s;
  uptime_s = HAL_GetTick() / 1000; /* the uptime in seconds */
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.StdId = 0x567;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 8;

  TxData[0] = (uint8_t)(uptime_s>>16);
  TxData[1] = (uint8_t)(uptime_s>>8);
  TxData[2] = (uint8_t)(uptime_s);
  TxData[3] = (uint8_t)(444>>8);
  TxData[4] = (uint8_t)(444);
  TxData[5] = 5;
  TxData[6] = 0xAF;
  TxData[7] = 0xFE;

  rc = HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
  if (rc != HAL_OK)
  {
   // Transmit did not work -> Error_Handler();
   //sprintf(strTmp, "HAL_CAN_AddTxMessage failed %ld", rc);
   //addToTrace(strTmp);
  } else {
   //sprintf(strTmp, "HAL_CAN_AddTxMessage ok for mailbox %ld", TxMailbox);
   //addToTrace(strTmp);
  }

}

void can_irq(CAN_HandleTypeDef *pcan) {
  nNumberOfCanInterrupts++;
  HAL_StatusTypeDef rc;
  rc = HAL_CAN_GetRxMessage(pcan, CAN_RX_FIFO0, &canRxMsgHdr, canRxData);
  if (rc==HAL_OK) {
    nNumberOfReceivedMessages++;
    canEvaluateReceivedMessage();
  }
}

void ADC_Select_CH0 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_0;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void ADC_Select_CH1 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_1;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int i;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  flashhandler_loadFromFlash();

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_CAN_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Init();
  HAL_ADC_Start(&hadc1);

  setKeepPowerOn(1); /* keep the power on */

  // Simple Text writing (Text, Font, X, Y, Color, BackColor)
  // Available Fonts are FONT1, FONT2, FONT3 and FONT4
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
  scheduler_init();
  can_init();
  powermodule_init();
  //Writing numbers
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


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
      //HAL_Delay(100);
	  scheduler_cyclic();
	  ADC_Select_CH0();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[0] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);

	  ADC_Select_CH1();
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  adcValues[1] = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);
      //canbus_demoTransmit();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  /* USER CODE BEGIN ADC1_Init 1 */

  uint8_t sConfig; /* This is to create a compile error ("conflicting types"), to detect the case
                      that the CubeMX generated this function new. For using the ADC in
                      single, manually triggered mode, we need to patch this function, by
                      removing all generated code. Afterwards, the compile error disappears. */

  /** Common config *** manually written. */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1; /* patched: we use 4 channels, but need to set 1 here. */
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */

  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 57;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_FilterTypeDef sf;
  sf.FilterMaskIdHigh = 0x0000;
  sf.FilterMaskIdLow = 0x0000;
  sf.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sf.FilterBank = 0;
  sf.FilterMode = CAN_FILTERMODE_IDMASK;
  sf.FilterScale = CAN_FILTERSCALE_32BIT;
  sf.FilterActivation = CAN_FILTER_ENABLE;
  if (HAL_CAN_ConfigFilter(&hcan, &sf) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, can_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_RegisterCallback(&hcan, HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID, can_mailbox0_complete_irq)) {
    Error_Handler();
  }
  if (HAL_CAN_Start(&hcan) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
    Error_Handler();
  }
  nNumberOfReceivedMessages=0;
  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_12
                          |GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB12
                           PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_12
                          |GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
