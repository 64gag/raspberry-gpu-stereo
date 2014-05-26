#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>

#define IMG_W 640
#define IMG_H 480
#define IMG_BPP 2
#define DEBUG 0

uint8_t image[IMG_W * IMG_H * IMG_BPP] = {0};


int readLine(unsigned int l)
{
	bcm2835_spi_transfern((char *)(&(image[l*IMG_W*IMG_BPP])), 15);//IMG_W*IMG_BPP);

	#if DEBUG
		int i;
		for(i=0; i<IMG_W*IMG_BPP; i++){
			printf("%X ", image[l*IMG_W*IMG_BPP+i]);
		}
	#endif
int i;
for(i = 0; i<15; i++)
	printf("%c", image[l*IMG_W*IMG_BPP+i]);
 return 0;
}


int main(int argc, char **argv)
{
	unsigned int i=0, j;
	if (!bcm2835_init()){
		return 1;
	}
	
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(6);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

//while(i++ < 60000){
	readLine(5);
//}



//uint8_t data = bcm2835_spi_transfer(0x23);
//printf("%02X", data);
	bcm2835_spi_end();
	bcm2835_close();
 return 0;
}
