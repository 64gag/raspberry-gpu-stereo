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

  /* DCMI config*/
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate= GPIO_AF13_DCMI;  
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  
  GPIO_Init_Structure.Pin =  GPIO_PIN_4 | GPIO_PIN_6;
   HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
   HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
   HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);
   
  /* EXTI lines */
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_1;
   HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_3;
   HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);
  GPIO_Init_Structure.Mode = GPIO_MODE_IT_RISING;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_4;
   HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);   

  /* SPI */
  GPIO_Init_Structure.Pin = GPIO_PIN_5; /* SCK */
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate = GPIO_AF5_SPI1;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);
  GPIO_Init_Structure.Pin = GPIO_PIN_4; /* MISO */
  GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Alternate = GPIO_AF5_SPI1;
  GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init_Structure.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);

  /* SCCB */ 
  GPIO_Init_Structure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Speed = GPIO_SPEED_FAST;
  GPIO_Init_Structure.Pin = GPIO_PIN_10 | GPIO_PIN_11;
   HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);
   
  /* This is to ensure CS line wired to the LIS3 remains up */
  GPIO_Init_Structure.Mode = GPIO_MODE_INPUT;
  GPIO_Init_Structure.Pull = GPIO_PULLUP;
  GPIO_Init_Structure.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);
  
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);    
}
