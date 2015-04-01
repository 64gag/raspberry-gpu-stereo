#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "sccb.h"
#include "debflags.h"

#define OV7670_SCCB_ADDRESS 0x21
#define FULL_DELAY 38
#define HALF_DELAY FULL_DELAY/2
#define SDIO_C 10 /* PD10 */
#define SDIO_D 11 /* PD11 */

#define SET_HIGH(PIN) (GPIOD->BSRRL = 1 << PIN)
#define SET_LOW(PIN) (GPIOD->BSRRH = 1 << PIN)
#define DATA_IN() (GPIOD->MODER = GPIOD->MODER & 0xffbfffff);
#define DATA_OUT() (GPIOD->MODER = GPIOD->MODER | 0x00400000)
#define DATA_HIGH() (GPIOD->IDR & 0x00000800)

volatile uint8_t val = 0;
uint8_t ovRegs[0xca];
void Delay(uint32_t d);

void Delay(uint32_t d){
  TIM6->ARR = d;
  TIM6->CNT = 0x0;
  TIM6->PSC = 0x8;
  TIM6->CR1 |=  /* ARR buffered */                           0<<7| \
                /* One-pulse mode */                         1<<3| \
                /* Update req. src (1 = only overflow) */    1<<2| \
                /* Update disable */                         0<<1| \
                /* Counter enable */                         1<<0;
  while(TIM6->CR1 & 0x1);
}

void SCCB_Init()
{
        __TIM6_CLK_ENABLE();

        SET_HIGH(SDIO_C);
	SET_HIGH(SDIO_D);

        SCCB_writeSlaveRegister(OV7670_SCCB_ADDRESS, 0x09, 0x03); /* Drive current */
        SCCB_writeSlaveRegister(OV7670_SCCB_ADDRESS, 0x3a, 0x04); /* YUYV sequence */
        SCCB_writeSlaveRegister(OV7670_SCCB_ADDRESS, 0x0c, 0x08); /* Scale enable */
        SCCB_writeSlaveRegister(OV7670_SCCB_ADDRESS, 0x12, 0x08); /* QCIF */
        
        __TIM6_CLK_DISABLE();
}

void SCCB_sendStart(void)
{
    	SET_HIGH(SDIO_D);
        Delay(FULL_DELAY);
	SET_HIGH(SDIO_C);
        Delay(FULL_DELAY);
	SET_LOW(SDIO_D);
        Delay(FULL_DELAY);
	SET_LOW(SDIO_C);
        Delay(HALF_DELAY);
}

void SCCB_sendStop(void)
{
	SET_LOW(SDIO_D);
        Delay(HALF_DELAY);
	SET_HIGH(SDIO_C);
        Delay(FULL_DELAY);
	SET_HIGH(SDIO_D);
        Delay(FULL_DELAY);
}


void SCCB_sendNACK(void)
{
	SET_HIGH(SDIO_D);
        Delay(HALF_DELAY);
	SET_HIGH(SDIO_C);
        Delay(FULL_DELAY);
	SET_LOW(SDIO_C);
        Delay(HALF_DELAY);
	SET_LOW(SDIO_D);
}

uint8_t SCCB_sendByte(uint8_t data)
{
	uint8_t sucess;

	for (uint8_t i = 0; i < 8; i++){
		if ((data << i) & 0x80){
			SET_HIGH(SDIO_D);
		}else{
			SET_LOW(SDIO_D);
		}
                Delay(HALF_DELAY);
		SET_HIGH(SDIO_C);
                Delay(FULL_DELAY);
		SET_LOW(SDIO_C);
                Delay(HALF_DELAY);
	}

	DATA_IN();
        Delay(HALF_DELAY);
	SET_HIGH(SDIO_C);
        Delay(FULL_DELAY);

	if (DATA_HIGH()){
		sucess = 0;
	}else{
		sucess = 1;
	}

	SET_LOW(SDIO_C);
        Delay(HALF_DELAY);
	DATA_OUT();

	return sucess;
}

uint8_t SCCB_getByte()
{
  uint8_t data = 0;
  DATA_IN();
  Delay(HALF_DELAY);

  for (uint8_t i = 8; i > 0; i--) {
    SET_HIGH(SDIO_C);
    Delay(FULL_DELAY);
    data = data << 1;
    if (DATA_HIGH())
      data++;

    SET_LOW(SDIO_C);
    Delay(HALF_DELAY);
    if(i != 1)
        Delay(HALF_DELAY);
  }

  DATA_OUT();

  return data;
}

uint8_t SCCB_writeSlaveRegister( uint8_t const slaveAddress, uint8_t const registerAddress, uint8_t const value)
{
	SCCB_sendStart();

	if (SCCB_sendByte(slaveAddress << 1)) {
		if (SCCB_sendByte(registerAddress)) {
			if (SCCB_sendByte(value)) {
				SCCB_sendStop();
				return 1;
			} else {
				SCCB_sendStop();
				return 0;
			}
		} else {
			SCCB_sendStop();
			return 0;
		}
	} else {
		SCCB_sendStop();
		return 0;
	}
}

uint8_t SCCB_readSlaveRegister(uint8_t const slaveAddress, uint8_t const registerAddress, volatile uint8_t *value)
{
 SCCB_sendStart();

	if (SCCB_sendByte(slaveAddress << 1)) {
		if (SCCB_sendByte(registerAddress)) {
			SCCB_sendStop();
			SCCB_sendStart();

			if (SCCB_sendByte((slaveAddress << 1) + 1)) {
				*value = SCCB_getByte();
				SCCB_sendNACK();
				SCCB_sendStop();
				return 1;
			} else {
				SCCB_sendStop();
				return 0;
			}
		} else {
			SCCB_sendStop();
			return 0;
		}
	} else {
		SCCB_sendStop();
		return 0;
	}
}
