#include "stm32f4xx_hal.h"
#include "debflags.h"

void GPIOInit(void);

void GPIOInit(void){
  GPIO_InitTypeDef GPIO_Init_Structure;

  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();  
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

  /* DCMI config*/
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate= GPIO_AF13_DCMI;  
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  
  GPIO_Init_Structure.Pin =  GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_9 | GPIO_PIN_10;
   HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_7;
   HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_8;
   HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_3;
   HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
   HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_11;
   HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
   
  /* EXTI lines */
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_RISING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_2;
   HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_3;
   HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_RISING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_4;
   HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);   

  /* SPI */
  GPIO_Init_Structure.Pin = GPIO_PIN_7; /* SCK */
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate = GPIO_AF5_SPI5;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_8; /* MISO */
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate = GPIO_AF5_SPI5;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

  /* SCCB and PWDN */
  GPIO_Init_Structure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
				/* SDIOC,	SDIOD,		PWDN */
  GPIO_Init_Structure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

  /* This is to ensure CS line wired to the gyro and LCD remain up */
  GPIO_Init_Structure.Mode = GPIO_MODE_INPUT;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_1 | GPIO_PIN_2;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
  
  /* This is to ensure LCD enable line wired remains low */
  GPIO_Init_Structure.Pull = GPIO_PULLDOWN;
  GPIO_Init_Structure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);
    
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);  
}

/**
  * @brief SDRAM MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param hsdram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
  GPIO_InitTypeDef  GPIO_Init_Structure;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clocks */
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  /* Enable FMC clock */
  __FMC_CLK_ENABLE();
                            
  /*##-2- Configure peripheral GPIO ##########################################*/  
/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+--------------------+--------------------+
 +                       SDRAM pins assignment                                      +
 +-------------------+--------------------+--------------------+--------------------+
 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   |
 | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   |
 | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG8  <-> FMC_SDCLK |
 | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG15 <-> FMC_NCAS  |
 | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    |--------------------+ 
 | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    |   
 | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  | 
 +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    | 
                     | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |    
                     | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
                     | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
 +-------------------+--------------------+--------------------+
 | PB5 <-> FMC_SDCKE1| 
 | PB6 <-> FMC_SDNE1 | 
 | PC0 <-> FMC_SDNWE |
 +-------------------+  
  
*/
  
  /* Common GPIO configuration */
  GPIO_Init_Structure.Mode  = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Speed = GPIO_SPEED_FAST;
  GPIO_Init_Structure.Pull  = GPIO_NOPULL;
  GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

  /* GPIOB configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);  

  /* GPIOC configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0;      
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);  
  
  /* GPIOD configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1  | GPIO_PIN_8 |
                                GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
                                GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

  /* GPIOE configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7 |
                                GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                                GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                                GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

  /* GPIOF configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0  | GPIO_PIN_1 | GPIO_PIN_2 | 
                                GPIO_PIN_3  | GPIO_PIN_4 | GPIO_PIN_5 |
                                GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                                GPIO_PIN_14 | GPIO_PIN_15;      
  HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

  /* GPIOG configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
                                GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);    
}

/**
  * @brief SDRAM MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to their default state
  * @param hsdram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
  /*## Disable peripherals and GPIO Clocks ###################################*/
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_6);
  
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
                  
  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1  | GPIO_PIN_8 |\
                         GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |\
                         GPIO_PIN_15);
 
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7 |\
                         GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |\
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |\
                         GPIO_PIN_14 | GPIO_PIN_15);
 
  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0  | GPIO_PIN_1 | GPIO_PIN_2 |\
                         GPIO_PIN_3  | GPIO_PIN_4 | GPIO_PIN_5 |\
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |\
                         GPIO_PIN_14 | GPIO_PIN_15);
 
  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |\
                         GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15);
}
