#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "led.h"
#include "debflags.h"
#include "dma.h"

uint32_t spi_accum = 0;
__IO uint32_t *DMA2S4_MAR[2] = {&(DMA2_Stream4->M0AR), &(DMA2_Stream4->M1AR)};
uint32_t lines = 0;
#ifndef NO_SDRAM
 extern uint32_t *dcmi_buff;
 uint8_t *dcmi_ptr = (uint8_t *)(SDRAM_BANK_ADDR + WRITE_READ_ADDR);
#else
 extern uint8_t dcmi_buff[];
 uint8_t *dcmi_ptr = &dcmi_buff[0];
#endif

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1);
}

void MemManage_Handler(void)
{
  while (1);
}

void BusFault_Handler(void)
{
  while (1);
}

void UsageFault_Handler(void)
{
  while (1);
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}


void SysTick_Handler(void)
{
  HAL_IncTick();
}

void DMA2_Stream1_IRQHandler(void)
{
  uint32_t FCR = 0x00;
  uint32_t SR = DMA2->LISR; /* Use a variable to avoid reading the volatile register (?) */
  
  if(SR & 0x340){  /* Some error */
    if(SR & 0x40){ /* Fifo error IF*/
      FCR |= 0x40;
    }

    if(SR & 0x100){ /* Direct mode error IF*/
      FCR |= 0x100;
    }

    if(SR & 0x200){ /* Stream transfer error IF*/
      FCR |= 0x200;
    }
  }

  if(SR & 0x400){ /* Half transfer IF*/
    FCR |= 0x400;
  }

  if(SR & 0x800){ /* Transfer complete IF*/
    FCR |= 0x800;
  }

  DMA2->LIFCR = FCR;
}

void DMA2_Stream4_IRQHandler(void)
{
  uint32_t FCR = 0x00;
  uint32_t SR = DMA2->HISR; /* Use a variable to avoid reading the volatile register (?) */

  #if DEB_INT  
  if(SR & 0x0d){  /* Some error */
    if(SR & 0x01){ /* Fifo error IF*/
      FCR |= 0x01;
    }

    if(SR & 0x04){ /* Direct mode error IF*/
      FCR |= 0x04;
    }

    if(SR & 0x08){ /* Stream transfer error IF*/
      FCR |= 0x08;
    }
  }

  if(SR & 0x20){ /* Transfer complete IF*/
    FCR |= 0x20;
  }
  #endif

  if(SR & 0x10){ /* Half transfer IF*/
    *DMA2S4_MAR[!(DMA2_Stream4->CR & 0x00080000)] = (uint32_t)(dcmi_ptr + (spi_accum + 1)*COMPLETE_NDTR);
    spi_accum++;
    FCR |= 0x10;
  }

  DMA2->HIFCR = FCR;
}


void DCMI_IRQHandler(void)
{
  uint32_t ICR = 0x00;
  uint32_t SR = DCMI->MISR; /* Use a variable to avoid reading the volatile register (?) */

  if(SR & 0x6){ /* Some error */
    if(SR & 0x2){       /* Overrun error */
      ICR |= 0x2;
    }

    if(SR & 0x4){       /* Sync error */
      ICR |= 0x4;
    }
  }

  if(SR & 0x1){       /* Frame */
    ICR |= 0x1;
  }
  
  if(SR & 0x8){       /* VSYNC */
    lines = 0;
    ICR |= 0x8;
  }
  
  if(SR & 0x10){      /* Line */
    lines++;
    ICR |= 0x10;
  }   

  DCMI->ICR = ICR;
}

/* FS falling */
void EXTI1_IRQHandler(void)
{
/* Not needed, coincides with CS falling */
}

/* FS rising */
void EXTI2_IRQHandler(void)
{
  GPIOC->BSRRH = 0x2000; /* Clear PWDN (PC13) */
  EXTI->PR = 0x04;
}

/* CS falling */
void EXTI3_IRQHandler(void)
{
  GPIOC->BSRRL = 0x2000; /* Set PWDN (PC13) */
  DcmiDma_Pause();
  Spi_Enable();
  EXTI->PR = 0x08;
}

/* CS rising */
void EXTI4_IRQHandler(void)
{
  spi_accum = 0;
  SpiDma_Reset();
  DcmiDma_Resume();
  EXTI->PR = 0x10;
}
