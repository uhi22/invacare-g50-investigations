# Hardware

* STM32F103C8T6 "Blue Pill"
* TFT display with ILI9341 controller

# Settings

* 8 MHz HSE
* 72 MHz SYSCLK
* 72 MHz HCLK
* 72 MHz HCLK to AHB
* 72 MHz to Cortex System Timer
* 72 MHz FCLK
* 36 MHz APB1 peri
* 72 MHz APB1 timer
* 36 MHz APB2 peri
* 72 MHz APB2 timer
* 9 MHz To ADC1,2

* CAN
    * Prescaler 57 (TQ 1583ns)
    * Seg 1: 3 times, Seg2: 2 times
    * SJW: 1 times
    * automatic bus-off recovery enabled
    * RX0 interrupt enabled
    * PA11 RX
    * PA12 TX
* SPI
    * Motorola, 8 bit, MSB first
    * Prescaler 2 (18 MBit)
    * CPOL low
    * CPHA 1 edge
    * DMA1 channel3 global interrupt enabled
    * DMA: SPI1TX : DMA1 channel 3, memory to peripheral, prio low.
    * PA5 SPI1_SCK
    * PA7 SPI_MOSI
    
