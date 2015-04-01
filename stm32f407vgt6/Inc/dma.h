#ifndef __DCMIDMA_H
#define __DCMIDMA_H

#include "debflags.h"
#include "stm32f4xx_hal.h"

#define COMPLETE_NDTR 0xffff

void DcmiDma_Init(void);
void DcmiDma_Pause(void);
void DcmiDma_Resume(void);

void SpiDma_Init(void);
void SpiDma_Reset(void);
void Spi_Enable(void);
void Spi_Disable(void);

#endif
