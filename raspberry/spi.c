#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <malloc.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#define DEBUG 0
#define PIN_READY RPI_V2_GPIO_P1_15
#define PIN_REQUEST RPI_V2_GPIO_P1_13

int readFrame(uint8_t *frameData, uint32_t frameSize, uint32_t options);

int main(int argc, char **argv)
{
	uint8_t *image;
	uint32_t width, height, divider=6, bpp=2, format=0, frames=10; /* Options and its defaults */
	uint32_t i, j, k; /* Loop counters */

	int o, index;
	opterr = 0;

	while ((o = getopt (argc, argv, ":p:w:h:d:f:b:")) != -1){
		switch (o)
		{
			case 'f':
				frames = atoi(optarg);
				break;
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
		}
	}

	if (!bcm2835_init()){
		fprintf(stderr, "bc2835_init() error!\n");
		return 1;
	}

	image = (uint8_t *)calloc(width*height*bpp+4, sizeof(uint8_t));
	if(image == NULL){
		fprintf (stderr, "Could not allocate memory to store the frames\n");
		return -1;
	}

	/* !Request pin configurations */
	bcm2835_gpio_fsel(PIN_REQUEST, BCM2835_GPIO_FSEL_OUTP);  /* Output */
	bcm2835_gpio_set_pud(PIN_REQUEST, BCM2835_GPIO_PUD_OFF); /* Not pulled by the pi */
	bcm2835_gpio_set(PIN_REQUEST);				 /* Set to high */

	/* !Ready pin configurations */
	bcm2835_gpio_fsel(PIN_READY, BCM2835_GPIO_FSEL_INPT);	 /* Input */
	bcm2835_gpio_set_pud(PIN_READY, BCM2835_GPIO_PUD_OFF);	 /* Not pulled by the pi */

	/* SPI settings */
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(divider);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

	/* Bug:
	 *  bcm2835_spi_begin()/end() "sessions" seem to shift the bits by one on each call 
	 * Workaround:
	 *  Create and destroy sessions until we receive properly aligned data
         */
	bcm2835_spi_begin();
	do{
		bcm2835_spi_end();
		bcm2835_spi_begin();
		bcm2835_gpio_clr(PIN_REQUEST); 		/* Signal the STM32 we gant a frame */
		while(bcm2835_gpio_lev(PIN_READY));	/* Wait some some microseconds until it is ready */
		bcm2835_gpio_set(PIN_REQUEST);		/* Signal the STM32 we know it is ready */
		readFrame(image, width*height*bpp, 0);	/* Get the frame */
	}while(image[0] != 0xcc);
	bcm2835_spi_begin();

	SDL_Surface *screen = NULL;
	SDL_Color colors[256];
        SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(width, height, 8, SDL_HWSURFACE);
	printf("\t%d\n",screen->pitch);

	for(i=0; i<256; i++){
		colors[i].r = i; colors[i].g = i; colors[i].b = i;
	}

	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);

	for(k=0; k<frames; k++){	
		uint16_t *p = (uint16_t*)(&image[5]);
		uint8_t *sp = (uint8_t*)screen->pixels;

		bcm2835_gpio_clr(PIN_REQUEST); 		/* Signal the STM32 we gant a frame */
		while(bcm2835_gpio_lev(PIN_READY));	/* Wait some some microseconds until it is ready */
		bcm2835_gpio_set(PIN_REQUEST);		/* Signal the STM32 we know it is ready */
		readFrame(image, width*height*bpp, 0);	/* Get the frame */

		for(i=0; i<height; i++){
			for(j=0; j<width; j++){
				*sp++ = (uint8_t)(*p++);
			}
			sp += 2;
		}
		SDL_Flip(screen);
	}

	/* Destroy, clean, free, etc */
	bcm2835_spi_end();	/* Properly aligned session finishes here, thank you! */
	bcm2835_close();
	SDL_FreeSurface(screen);
	SDL_Quit();
	free(image);

 return 0;
}


int readFrame(uint8_t *frameData, uint32_t frameSize, uint32_t options)
{
	bcm2835_spi_transfern((char *)frameData, frameSize+4); /* 4 extra bytes */

	if(options){	
		unsigned int i;
		for(i = 0; i<frameSize; i++){
			printf("%02X ", *(frameData++));
			if(i%options == options-1){
				printf("\n");
			}
		}
		printf("\n");
	}

 return 0;
}
