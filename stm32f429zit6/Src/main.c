#include "debflags.h"
#include "stm32f4xx_hal.h"

#include "sccb.h"
#include "dma.h"
#include "led.h"
#ifndef NO_SDRAM
 #include "sdram.h"
#endif
 
#ifndef NO_SDRAM
  uint32_t *dcmi_buff = (uint32_t*)(SDRAM_BANK_ADDR + WRITE_READ_ADDR);
#else
  uint8_t dcmi_buff[IMG_W*IMG_H*IMG_BPP];
#endif

uint32_t freq = 0;

static void Clocks_Config(void);
void GPIOInit(void);

#ifndef NO_SDRAM
  static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
  static void SDRAM_Config(void);
#endif

int main(void)
{
  HAL_Init();

  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);

  Clocks_Config(); /* Configures sysclock at 180MHz (HSE+PLL) and MCO to 24MHz (I2SPLL) */
  

  #ifndef NO_SDRAM
  SDRAM_Config();
  #endif

  GPIOInit();
  SCCB_Init();
  DcmiDma_Init();
  SpiDma_Init();
 
  while(1);
}

static void Clocks_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

#ifdef PLL_I2S
        RCC_PeriphCLKInitTypeDef RCC_PClockInitStruct;
        HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLLI2SCLK, RCC_MCODIV_4);
#else
        HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSE, RCC_MCODIV_2);
#endif

	/* Enable Power Control clock */
	__PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Activate the Over Drive feature (available only for STM32F42xxx/43xxx devices)*/
	HAL_PWREx_ActivateOverDrive();

#ifdef PLL_I2S
	RCC_PClockInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	RCC_PClockInitStruct.PLLI2S.PLLI2SN = 265;
	RCC_PClockInitStruct.PLLI2S.PLLI2SR = 2;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PClockInitStruct);
#endif
      
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  while (1){}
}
#endif
