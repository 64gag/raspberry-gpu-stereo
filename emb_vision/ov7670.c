#include "emb_vision.h"
#include <stdio.h>
#include "ov7670.h"

#define PIN_FSX RPI_V2_GPIO_P1_11
#define PIN_CSL RPI_V2_GPIO_P1_13
#define PIN_CSR RPI_V2_GPIO_P1_15

#define PIN_MISO RPI_V2_GPIO_P1_21
#define PIN_SCK RPI_V2_GPIO_P1_23

void OV_ReadFrames(uint8_t *frameLeft, uint8_t *frameRight, uint32_t *clock_div)
{
		bcm2835_spi_setClockDivider(clock_div[CAM_LEFT]);
		bcm2835_gpio_clr(PIN_FSX);
		bcm2835_gpio_clr(PIN_CSL);
			bcm2835_spi_transfern(frameLeft, TEX_SIZE);
		bcm2835_gpio_set(PIN_CSL);

		bcm2835_spi_setClockDivider(clock_div[CAM_RIGHT]);
		bcm2835_gpio_clr(PIN_CSR);
//			bcm2835_spi_transfern(frameRight, TEX_SIZE);
		bcm2835_gpio_set(PIN_CSR);
		bcm2835_gpio_set(PIN_FSX);
}

void OV_InitCommunication(void)
{
	if (!bcm2835_init()){
		return;
	}

	bcm2835_gpio_fsel(PIN_FSX, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set_pud(PIN_FSX, BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_set(PIN_FSX);

	bcm2835_gpio_fsel(PIN_CSL, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set_pud(PIN_CSL, BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_set(PIN_CSL);

	bcm2835_gpio_fsel(PIN_CSR, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set_pud(PIN_CSR, BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_set(PIN_CSR);

	bcm2835_gpio_set_pud(PIN_SCK, BCM2835_GPIO_PUD_UP);

	/* SPI settings (default?) */
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

	bcm2835_spi_begin();
}

void OV_EndCommunication(void)
{
	bcm2835_spi_end();
	bcm2835_close();
}
