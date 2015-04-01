#include "buildflags.h"
#if SDL_USED
 #include <SDL/SDL.h>
 #include <SDL/SDL_gfxPrimitives.h>
#endif
#include "prob_hough.h"
#include <time.h>

const int img_w = IMG_W; /* Should change to use these later, focusing on speed right now */
const int img_h = IMG_H;

int main( int argc, char* args[] )
{
	img_t in, gauss, out;

	phough_t HT = {0};
	lines_t lv = {0};
	roi_t ROI = {0,0, 640, 480};
	HT.l = &lv;
	in.w = out.w = gauss.w = IMG_W;
	in.h = out.h = gauss.h = IMG_H;
	in.r = out.r = gauss.r = &ROI;

	#if STATIC_ALLOC
		unsigned char in_img_data[IMG_W * IMG_H] = {0};
		unsigned char gauss_img_data[IMG_W * IMG_H] = {0};
		unsigned char out_img_data[IMG_W * IMG_H] = {0};
		unsigned int hough_accumulator[((IMG_W + IMG_H) * 2 + 1)*180]; //rho res ignored!
		in.d = in_img_data;
		gauss.d = gauss_img_data;
		out.d = out_img_data;
		HT.accu = hough_accumulator;
	#else
		in.d = malloc(in.w * in.h * sizeof(unsigned char));
		gauss.d = malloc(gauss.w * gauss.h * sizeof(unsigned char));
		out.d = malloc(out.w * out.h * sizeof(unsigned char));
		HT.accu = malloc((int)((IMG_W + IMG_H) * 2 + 1)*180 * sizeof(unsigned int));
	#endif

	#if DEBUG
		printf("%d", ((IMG_W + IMG_H) * 2 + 1)*180); /* Size of allocated memory blocks */
	#endif

	#if SANITY_CHECKS
		if(in.d && gauss.d && out.d){
			#if DEBUG
				printf("memory OK\n");
			#endif
		}
	#endif

	#if SDL_USED
		SDL_Surface* screen = NULL;
		SDL_Init(SDL_INIT_EVERYTHING);
	#endif
	#if LOAD_BMP_SDL
		SDL_Surface* input_img = NULL;
		screen = SDL_SetVideoMode(IMG_W, IMG_H, 32, SDL_HWSURFACE);
		input_img = SDL_LoadBMP("input.bmp");
		SDL_BlitSurface(input_img, NULL, screen, NULL);
		{
			unsigned int i, j;
			Uint32 p;
			for(i=0; i<IMG_H; i++){
				for(j=0; j<IMG_W; j++){
					in.d[i*in.w + j] = (unsigned char)get_pixel32(screen, j, i);
				}
			}
		}
	#else
	
	#endif


	unsigned int x;

	#if PRINT_TIME
		clock_t start = clock();
	#endif

	for(x = 0; x<ITERATIONS; x++){
		gaussian_noise_reduce(&in, &gauss);	/* Pre-edge detection: some blurring */
		canny_edge_detect(&gauss, &out);	/* Actual edge detection */
		#if DRAW_OUTPUT
			screen = SDL_SetVideoMode(IMG_W, IMG_H, 32, SDL_HWSURFACE);
			{
				unsigned int i, j;
				Uint32 p;
				for(i=0; i<IMG_H; i++){
					for(j=0; j<IMG_W; j++){
						p = out.d[i*out.w + j];
						put_pixel32(screen, j, i, (0xff000000 | (p << 16) | (p << 8) | p));
					}
				}

			}
		#endif

		init_hough(&HT);
		phough_transform(&out, &HT); /* Transform */

		#if DEBUG
			printf("Accumulator info:\n \timage \tw: %d, h: %d\n", HT.img_w, HT.img_h);
			printf("\taccum \tw: %d, h: %d, max: %d\n", HT.w, HT.h, HT.max);
		#endif

		#if DRAW_OUTPUT
			{
				int i;

				for(i=0; i<HT.l->count; i++){ /* Draw all the detected lines */
					lineRGBA(screen, HT.l->l[i].x1, HT.l->l[i].y1, HT.l->l[i].x2, HT.l->l[i].y2, 0xff, 0, 0, 0xff);
				}
			}

			SDL_Flip(screen);
			#if WAIT_KEY
				wait();
			#endif
		#endif
	}

	#if PRINT_TIME
		printf("Hough transform - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif

	#if DRAW_ACCUM /* Draw the accumulator */
		screen = SDL_SetVideoMode(HT.w, HT.h, 32, SDL_HWSURFACE); /* Set the window to accumulator's size */
		{
			unsigned int i, j;
			Uint32 p;
			for(i=0; i<HT.h; i++){ /* Loop through every pixel */
				for(j=0; j<HT.w; j++){
					p = 0xff*(1-(float)HT.accu[i*HT.w + j]/(float)HT.max); /* Maximize contrast */
					put_pixel32(screen, j, i, (0xff000000 | (p << 16) | (p << 8) | p)); /* Draw each pixel */
				}
			}
		}
		SDL_Flip(screen); /* Flush to window */

		#if WAIT_KEY
			wait();
		#endif
	#endif

	#if LOAD_BMP_SDL
		SDL_FreeSurface(input_img);
	#endif

	#if SDL_USED
		SDL_FreeSurface(screen);
		SDL_Quit();
	#endif

	#if !STATIC_ALLOC
		free(in.d);
		free(gauss.d);
		free(out.d);
		free(HT.accu);
	#endif

 return 0;
}

#if SDL_USED
 inline Uint32 get_pixel32( SDL_Surface *surface, int x, int y )
 { 
	Uint32 *pixels = (Uint32 *)surface->pixels;
	return pixels[ ( y * surface->w ) + x ];
 }

 inline void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
 {
	Uint32 *pixels = (Uint32 *)surface->pixels;
	pixels[ ( y * surface->w ) + x ] = pixel;
 }

 inline void wait()
 {
	SDL_Event event;

	while(1) {
		SDL_PollEvent( &event );
		if(event.type == SDL_KEYDOWN)
			break;
	}
 }
#endif
