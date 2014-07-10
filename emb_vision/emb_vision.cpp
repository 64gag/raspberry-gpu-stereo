#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

#include "graphics.h"
#include "emb_vision.h"

#define DEBUG 0

#if CAM
 #include "ov7670.h"
 /* Memory to store the frames */
 uint8_t frame_data_l[TEX_SIZE];
 uint8_t frame_data_r[TEX_SIZE];
#else
 #include "img.h"
#endif

/* Prototypes */
void *Capture(void *t);
void *Depth(void *t);
void *Lane(void *t);

/* Pointer to array of pointers to functions */
void *(*f_thread[3])(void *t) = {Capture, Depth, Lane};

/* Synchronization variables */
uint8_t ready_threads = 0;
pthread_mutex_t ready_threads_mutex=PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t ready_threads_cv=PTHREAD_COND_INITIALIZER; 
uint8_t frame = 0;
pthread_mutex_t frame_mutex=PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t frame_cv=PTHREAD_COND_INITIALIZER; 
uint8_t capture=0;
pthread_mutex_t capture_mutex=PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t capture_cv=PTHREAD_COND_INITIALIZER; 
volatile uint8_t run = 1;

/* This is called when ctrl+c is pressed */
void sigintHandler(int s)
{
	run = 0; /* Break out of loops, threads! */
}

int main(int argc, char **argv)
{
	pthread_t thread[THREAD_COUNT];
	pthread_attr_t attr;
	struct sched_param priority_param;
	int rc;

	uint32_t arguments[ARG_COUNT] = {8, 0, 100, 0, 100000};

	signal(SIGINT, sigintHandler);	/* Point the signal to the handler /

	/* Commandline arguments and options handling */
	int o, index;
	opterr = 0;
	while ((o = getopt (argc, argv, ":l:r:")) != -1){
		switch (o)
		{
			case 'l':
				arguments[ARG_SPI_DIV_L] = atoi(optarg);
				break;
			case 'r':
				arguments[ARG_SPI_DIV_R] = atoi(optarg);
				break;
			case 'd':
				arguments[ARG_TRAN_DELAY_L] = atoi(optarg);
				break;
			case 'g'
				arguments[ARG_TRAN_DELAY_R] = atoi(optarg);
				break;
			case 'f':
				arguments[ARG_FRAME_DELAY] = atoi(optarg);
				break;
			case ':':
				fprintf(stderr, "No argument specified to option -%c.\n", optopt);
				return -1;
				break;
			default:
				fprintf(stderr, "Unexpected option -%c. Is this OK?\n", optopt);
		}
	}

	if(!arguments[ARG_TRAN_DELAY_R]){
		arguments[ARG_TRAN_DELAY_R] = arguments[ARG_TRAN_DELAY_L];
	}

	if(!arguments[ARG_SPI_DIV_R]){
		arguments[ARG_SPI_DIV_R] = arguments[ARG_SPI_DIV_L];
	}

	/* Initialize attribute and set thread detach state to joinable */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* Main thread: lower priority than capture thread */
	priority_param.sched_priority = sched_get_priority_max(SCHED_RR) - 1;
	pthread_setschedparam(pthread_self(), SCHED_RR, &priority_param);

	/* Capture thread: max priority */
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	priority_param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr, &priority_param);
	pthread_create(&thread[THREAD_CAPTURE], &attr, f_thread[THREAD_CAPTURE], (void *)arguments);

	/* Processing threads: lower priority than capture thread */
	priority_param.sched_priority = sched_get_priority_max(SCHED_RR) - 1;
	pthread_attr_setschedparam(&attr, &priority_param);
	for(int t = 1; t < THREAD_COUNT; t++) {
		pthread_create(&thread[t], &attr, f_thread[t], (void *)arguments);
	}

	/* Destroy attribute */
	pthread_attr_destroy(&attr);

	while(run){
		/* Request the capture thread a frame */
		pthread_mutex_lock(&capture_mutex);
		capture = 1;
		pthread_cond_broadcast(&capture_cv); 
		pthread_mutex_unlock(&capture_mutex);

		/* Lock until capture is done */
		pthread_mutex_lock(&capture_mutex);
		while(run && capture){
			pthread_cond_wait(&capture_cv, &capture_mutex);
		}			
		pthread_mutex_unlock(&capture_mutex);

		usleep(arguments[ARG_FRAME_DELAY]);
	}

	/* Kick the processing threads out of locked state */
	pthread_mutex_lock(&frame_mutex);
	frame = 2;
	pthread_cond_broadcast(&frame_cv);
	pthread_mutex_unlock(&frame_mutex);

	/* Kick the capture thread out of locked state */
	pthread_mutex_lock(&capture_mutex);
	capture = 1;
	pthread_cond_broadcast(&capture_cv);
	pthread_mutex_unlock(&capture_mutex);


	/* Wait for the other threads to finish */
	for(int t = 0; t < THREAD_COUNT; t++) {
		rc = pthread_join(thread[t], NULL);
		if (rc) {
			#if DEBUG
				printf("pthread_join() : %d\n", rc);
			#endif
			exit(-1);
		}
	}
	#if DEBUG
		printf("All threads completed. Exiting.\n");
	#endif

 return 0;
}

void *Capture(void *t)
{
	#if CAM
	OV_InitCommunication();
	#endif

	uint32_t *args = (uint32_t *)t;

	/* Make sure all threads are ready before starting */
	#if DEBUG
		printf("Capture() ready!\n");
	#endif
	pthread_mutex_lock(&ready_threads_mutex);
	if(++ready_threads <  THREAD_COUNT){
		while(ready_threads < THREAD_COUNT){
			pthread_cond_wait(&ready_threads_cv, &ready_threads_mutex);
		}
	}else{
		pthread_cond_broadcast(&ready_threads_cv); 
	}
	pthread_mutex_unlock(&ready_threads_mutex);

	while(run){
		/* Lock until signaled to capture */
		pthread_mutex_lock(&capture_mutex);
		while(!capture){
			pthread_cond_wait(&capture_cv, &capture_mutex);
		}			
		pthread_mutex_unlock(&capture_mutex);

		/* Did the party finish while I was sleeping? :@ */
		if(!run){
			break;
		}

		#if CAM
		OV_ReadFrames(frame_data_l, frame_data_r, args);
		#endif

		/* Let processing threads know a frame is ready */
		pthread_mutex_lock(&frame_mutex);
		frame = THREAD_COUNT - 1; /* How many threads care? */
		#if DEBUG
			printf("Frame captured.\n");
		#endif
		pthread_cond_broadcast(&frame_cv); 
		pthread_mutex_unlock(&frame_mutex);

		/* Let main thread know the capture is done */
		pthread_mutex_lock(&capture_mutex);
		capture = 0;
		pthread_cond_broadcast(&capture_cv); 
		pthread_mutex_unlock(&capture_mutex);
	}

	#if CAM
	OV_EndCommunication();
	#endif

 pthread_exit(NULL);
}

void *Depth(void *t)
{
	InitGraphics();

	/* Textures for each program */
	GfxTexture textures[GRID_COLS*GRID_ROWS];

	/* Inputs are greyscale */
	textures[TEX_LIRAW].CreateGreyScale(TEX_W, TEX_H);
	textures[TEX_RIRAW].CreateGreyScale(TEX_W, TEX_H);

	/* The rest are encoded RGBA (to be able to write to them) */
	for(int i = TEX_LHENC; i < TEX_HDEC; i++){
		textures[i].CreateRGBA(TEX_W/4, TEX_H);	/* Encoded! divide by 4 */
		textures[i].GenerateFrameBuffer();
	}
	textures[TEX_HDEC].CreateRGBA(TEX_W, TEX_H); /* This is not encoded, visualize output here */
	textures[TEX_HDEC].GenerateFrameBuffer();

	/* Array of pointers to "support" grid logic */
	GfxTexture* tex_grid[GRID_COLS*GRID_ROWS];
	for(int i = 0; i < GRID_COLS * GRID_ROWS; i++){
		tex_grid[i] = &textures[i];
	}

	int selected_texture = TEX_GRID;

	/* Scale grid to fill the screen */
	float grid_col_size = 2.f / GRID_COLS;
	float grid_row_size = 2.f / GRID_ROWS;

	/* Make sure all threads are ready before starting */
	#if DEBUG
		printf("Depth() ready!\n");
	#endif
	pthread_mutex_lock(&ready_threads_mutex);
	if(++ready_threads <  THREAD_COUNT){
		while(ready_threads < THREAD_COUNT){
			pthread_cond_wait(&ready_threads_cv, &ready_threads_mutex);
		}
	}else{
		pthread_cond_broadcast(&ready_threads_cv); 
	}
	pthread_mutex_unlock(&ready_threads_mutex);

	while(run){
		/* Lock until we have a new frame */
		pthread_mutex_lock(&frame_mutex);
		while(!frame){
			pthread_cond_wait(&frame_cv, &frame_mutex);
		}			
		pthread_mutex_unlock(&frame_mutex);

		/* Did the party finish while I was sleeping? :@ */
		if(!run){
			break;
		}

		#if CAM
		textures[TEX_LIRAW].SetPixels(frame_data_l);
		textures[TEX_RIRAW].SetPixels(frame_data_r);
		#else
		textures[TEX_LIRAW].SetPixels(frame_data_l);
		textures[TEX_RIRAW].SetPixels(frame_data_r);
		#endif

		/* Let other threads know this thread is done READING current frame */
		pthread_mutex_lock(&frame_mutex);
		frame--;
		#if DEBUG
			printf("\tDepth done...\n");
		#endif
		pthread_mutex_unlock(&frame_mutex);

//		selected_texture = (selected_texture + 1) % (GRID_ROWS * GRID_COLS);

		BeginFrame();
			/* First required textures are drawn to frame buffers */
			if(selected_texture == TEX_GRID || &textures[TEX_LHENC] == tex_grid[selected_texture]){
				DrawTextureRect(&textures[TEX_LIRAW], -1.f, -1.f, 1.f, 1.f, &textures[TEX_LHENC], SHAD_HENC);
			}

			if(selected_texture == TEX_GRID || &textures[TEX_LSWIN] == tex_grid[selected_texture]){
				DrawTextureRect(&textures[TEX_LHENC], -1.f, -1.f, 1.f, 1.f, &textures[TEX_LSWIN], SHAD_SWIN);
			}

			if(selected_texture == TEX_GRID || &textures[TEX_RHENC] == tex_grid[selected_texture]){
				DrawTextureRect(&textures[TEX_RIRAW], -1.f, -1.f, 1.f, 1.f, &textures[TEX_RHENC], SHAD_HENC);
			}

			if(selected_texture == TEX_GRID || &textures[TEX_RSWIN] == tex_grid[selected_texture]){
				DrawTextureRect(&textures[TEX_RHENC], -1.f, -1.f, 1.f, 1.f, &textures[TEX_RSWIN], SHAD_SWIN);
			}

			if(selected_texture == TEX_GRID || &textures[TEX_DMAP] == tex_grid[selected_texture]){
				DrawDisparityRect(&textures[TEX_LSWIN], &textures[TEX_RSWIN], &textures[TEX_DMAP], 0, -1.f, -1.f, 1.f, 1.f);
			}

			if(selected_texture == TEX_GRID || &textures[TEX_HDEC] == tex_grid[selected_texture]){
				DrawTextureRect(&textures[TEX_DMAP], -1.f, -1.f, 1.f, 1.f, &textures[TEX_HDEC], SHAD_HDEC);
			}

			/* Generated frame buffers are drawn to screen */
			GfxTexture* tex;
			if(selected_texture == TEX_GRID){ /* Draw grid */
				for(int col = 0; col < GRID_COLS; col++){
					for(int row = 0; row < GRID_ROWS; row++){
						tex = tex_grid[col + row * GRID_COLS];
						float colx = -1.f + col * grid_col_size;
						float rowy = -1.f + row * grid_row_size;
						DrawTextureRect(tex, colx, rowy , colx + grid_col_size, rowy + grid_row_size, NULL, SHAD_IRAW);
					}
				}
			}else{ /* Draw single texture */
				tex = tex_grid[selected_texture];
				DrawTextureRect(tex, -1.0f, -1.0f, 1.0f, 1.0f, NULL, SHAD_IRAW);
			}
		EndFrame();
	}

 pthread_exit(NULL);
}

void *Lane(void *t)
{
	/* Make sure all threads are ready before starting */
	#if DEBUG
		printf("Lane() ready!\n");
	#endif
	pthread_mutex_lock(&ready_threads_mutex);
	if(++ready_threads <  THREAD_COUNT){
		while(ready_threads < THREAD_COUNT){
			pthread_cond_wait(&ready_threads_cv, &ready_threads_mutex);
		}
	}else{
		pthread_cond_broadcast(&ready_threads_cv); 
	}
	pthread_mutex_unlock(&ready_threads_mutex);

	while(run){
		/* Lock until we have a new frame */
		pthread_mutex_lock(&frame_mutex);
		while(!frame){
			pthread_cond_wait(&frame_cv, &frame_mutex);
		}			
		pthread_mutex_unlock(&frame_mutex);

		/* Did the party finish while I was sleeping? :@ */
		if(!run){
			break;
		}

		/* Let other threads know this thread is done with current frame */
		pthread_mutex_lock(&frame_mutex);
		frame--;
		#if DEBUG
			printf("\tLane done...\n");
		#endif
		pthread_mutex_unlock(&frame_mutex);

	}

 pthread_exit((void*) t);
}
