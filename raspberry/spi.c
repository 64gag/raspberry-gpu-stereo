#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <malloc.h>

int readFrame(uint8_t *frameData, uint32_t frameSize, uint32_t options)
{
	bcm2835_spi_transfern((char *)frameData, frameSize);

	if(options){	
		unsigned int i;
		for(i = 0; i<frameSize; i++){
			printf("%02X ", *(frameData++));
			if(i%options == options-1){
				printf("\n");
			}
		}
	}

 return 0;
}

int main(int argc, char **argv)
{
 uint8_t *image;
 uint32_t width, height, divider=8, bpp=2, format=0;

	int o, index;
	opterr = 0;

	while ((o = getopt (argc, argv, ":p:w:h:d:b:")) != -1){
		switch (o)
		{
			case 'p':
				format = atoi(optarg);
				break;
			case 'w':
				width = atoi(optarg);
				break;
			case 'h':
				height = atoi(optarg);
				break;
			case 'd':
				divider = atoi(optarg);
				break;
			case 'b':
				bpp = atoi(optarg);
				break;
			case ':':
				fprintf(stderr, "No argument specified to option -%c.\n", optopt);
				return -1;
			default:
				fprintf(stderr, "Unexpected option -%c. Is this OK?\n", optopt);
				fprintf(stderr, "\tUsage spi -p<Print block size> -w<Width of frame> - h<Height of frame> -d<Divider for the SPI clock> -b<Bytes per pixel>\n\n");
		}
	}

	if (!bcm2835_init()){
		fprintf(stderr, "bc2835_init() error!\n");
		return 1;
	}

	image = (uint8_t *)calloc(width*height*bpp, sizeof(uint8_t));
	if(image == NULL){
		fprintf (stderr, "Could not allocate memory to store the frames\n");
		return -1;
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(divider);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

	readFrame(image, width*height*bpp, format);

	bcm2835_spi_end();
	bcm2835_close();

	free(image);
 return 0;
}
