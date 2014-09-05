#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>
#include <vector>

#include "graphics.h"
#include "emb_vision.h"

#define DEBUG 0

#if CAM
 #include "ov7670.h"
 /* Memory to store the frames */
 uint8_t left_frame_data[IMG_SIZE];
 uint8_t right_frame_data[IMG_SIZE];
#else
 #include "left_frame_data.h"
 #include "right_frame_data.h"
#endif

extern uint32_t GScreenWidth;
extern uint32_t GScreenHeight;
extern EGLDisplay GDisplay;
extern EGLSurface GSurface;
extern EGLContext GContext;

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

	uint32_t arguments[ARG_COUNT] = {8, 0, 100, 0, 100000, 2};

	signal(SIGINT, sigintHandler);	/* Point the signal to the handler /

	/* Commandline arguments and options handling */
	int o, index;
	opterr = 0;
	while ((o = getopt (argc, argv, ":l:r:d:g:f:t:")) != -1){
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
			case 'g':
				arguments[ARG_TRAN_DELAY_R] = atoi(optarg);
				break;
			case 'f':
				arguments[ARG_FRAME_DELAY] = atoi(optarg);
				break;
			case 't':
				arguments[ARG_DRAWN_TEX] = atoi(optarg);
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
		OV_ReadFrames(left_frame_data, right_frame_data, args);
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
	std::vector<GLTexture> textures;
	std::vector<GLShader> shaders;
	std::vector<GLProgram> programs;

	GLint k_location;

	InitGraphics(textures, shaders, programs);

	/* Set offset and scale uniforms, get texelsize locations */
	for(std::vector<GLProgram>::size_type i = 0; i < programs.size(); i++) {
		glUseProgram(programs[i].id());
		glUniform2f(glGetUniformLocation(programs[i].id(), "offset"), -1.0f, -1.0f);
		glUniform2f(glGetUniformLocation(programs[i].id(), "scale"), 2.0f, 2.0f);
		glVertexAttribPointer(glGetAttribLocation(programs[i].id(), "vertex"), 4, GL_FLOAT, 0, 16, 0);
		glEnableVertexAttribArray(glGetAttribLocation(programs[i].id(), "vertex"));
		programs[i].set_texel_location(glGetUniformLocation(programs[i].id(), "texelsize"));
	}

	/* Set constant texelsizes */
	glUseProgram(programs[FS_PACK].id());
	glUniform2f(programs[FS_PACK].id(), 1.0f/textures[TEX_LEFT].width(), 1.0f/textures[TEX_LEFT].height());
	glUseProgram(programs[FS_DMAP0].id());
	glUniform2f(programs[FS_DMAP0].texel_location(), 1.0f/textures[TEX_DMAP16].width(), 1.0f/textures[TEX_DMAP16].height());
	glUseProgram(programs[FS_DILATE].id());
	glUniform2f(programs[FS_DILATE].texel_location(), 1.0f/128.0f, 1.0f/128.0f);

	/* Few other constant uniforms */
	glUseProgram(programs[FS_PACK].id());
	glUniform2f(glGetUniformLocation(programs[FS_PACK].id(), "range"), 256.0f/textures[0].width(), 256.0f/textures[0].height());
	glUniform2f(glGetUniformLocation(programs[FS_PACK].id(), "from"), (textures[0].width() - 256.0f) / (2.0f * textures[0].width()),
									  (textures[0].height() - 256.0f) / (2.0f * textures[0].height()));
	glUniform1i(glGetUniformLocation(programs[FS_PACK].id(), "G"), 1);
	glUseProgram(programs[FS_DMAPN].id());
	glUniform1i(glGetUniformLocation(programs[FS_DMAPN].id(), "dmap"), 1);

	/* Save location of k */
	k_location = glGetUniformLocation(programs[FS_CONTRAST].id(), "k");

	glBindTexture(GL_TEXTURE_2D, textures[TEX_STEREO].id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);

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

	/* Print information of interest */
	int gl_val;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_val);
	std::cout << "MAX_TEXTURE_IMAGE_UNITS: " << gl_val << std::endl;
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &gl_val);
	std::cout << "MAX_VARYING_VECTORS: " << gl_val << std::endl;

	/* Read start time */
	long int start_time;
	long int time_difference;
	struct timespec gettime_now;
	clock_gettime(CLOCK_REALTIME, &gettime_now);
	start_time = gettime_now.tv_nsec ;
	double total_time_s = 0;
	int cnt = 0;

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

		textures[TEX_LEFT].SetPixels(left_frame_data);
		textures[TEX_RIGHT].SetPixels(right_frame_data);

		/* Let other threads know this thread is done READING current frame */
		pthread_mutex_lock(&frame_mutex);
		frame--;
		#if DEBUG
			printf("\tDepth done...\n");
		#endif
		pthread_mutex_unlock(&frame_mutex);

		/* Capture POT region and pack to a single texture */
		glUseProgram(programs[FS_PACK].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[TEX_STEREO].framebuffer_id());
		glViewport(0, 0, textures[TEX_STEREO].width(), textures[TEX_STEREO].height());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[TEX_LEFT].id());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[TEX_RIGHT].id());
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Generate the mipmaps */
		glBindTexture(GL_TEXTURE_2D, textures[TEX_STEREO].id());
		glGenerateMipmap(GL_TEXTURE_2D);

		/* Lowest resolution pass */
		glUseProgram(programs[FS_DMAP0].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[TEX_DMAP16].framebuffer_id());
		glViewport(0, 0, textures[TEX_DMAP16].width(), textures[TEX_DMAP16].height());
		glBindTexture(GL_TEXTURE_2D, textures[TEX_STEREO].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Grow the previous disparity map, calculate new disparities, loop */
		for(int level = 0; level < 4; level++){
			/* Grow the previous */
			int grow_index = TEX_DMAP16 + level;
			int grown_index = grow_index + 1;
			int new_index = grown_index;

			glUseProgram(programs[FS_DINTERPOLATE].id());
			glUniform2f(programs[FS_DINTERPOLATE].texel_location(), 1.0f/textures[grow_index].width(), 1.0f/textures[grow_index].height());
			glBindFramebuffer(GL_FRAMEBUFFER, textures[grown_index].framebuffer_id());
			glViewport(0, 0, textures[grown_index].width(), textures[grown_index].height());
			glBindTexture(GL_TEXTURE_2D, textures[grow_index].id());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glFlush(); /* The driver ditches the interpolation if you do not flush */

			/* Calculate new disparities */
			glUseProgram(programs[FS_DMAPN].id());
			glUniform2f(programs[FS_DMAPN].texel_location(), 1.0f/textures[grown_index].width(), 1.0f/textures[grown_index].height());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[TEX_STEREO].id());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[grown_index].id());
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		/* Increase contrast for visualization */
		glUseProgram(programs[FS_CONTRAST].id());
		for(int level = 0; level <= 4; level++){
			int contrast_index = TEX_DMAP16 + level;
			glUniform1f(k_location, (float)(6*(5-level)));
			glUniform2f(programs[FS_CONTRAST].texel_location(), 1.0f/textures[contrast_index].width(), 1.0f/textures[contrast_index].height());
			glBindFramebuffer(GL_FRAMEBUFFER, textures[contrast_index].framebuffer_id());
			glViewport(0, 0, textures[contrast_index].width(), textures[contrast_index].height());
			glBindTexture(GL_TEXTURE_2D, textures[contrast_index].id());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		/* Filter to remove noise from inaccurate match */
		glUseProgram(programs[FS_DILATE].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[TEX_FILTER].framebuffer_id());
		glViewport(0, 0, textures[TEX_FILTER].width(), textures[TEX_FILTER].height());
		glBindTexture(GL_TEXTURE_2D, textures[TEX_DMAP256].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Calculate the maximum by row of 8 elements */
		glUseProgram(programs[FS_ERODE].id());
		glUniform2f(programs[FS_ERODE].texel_location(), 1.0f/textures[TEX_FILTER].width(), 0.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, textures[TEX_MAX_HOR].framebuffer_id());
		glViewport(0, 0, textures[TEX_MAX_HOR].width(), textures[TEX_MAX_HOR].height());
		glBindTexture(GL_TEXTURE_2D, textures[TEX_FILTER].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Calculate the maximum by column of 8 elements */
		glUniform2f(programs[FS_ERODE].texel_location(), 0.0f, 1.0f/textures[TEX_MAX_HOR].height());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[TEX_MAX_VER].framebuffer_id());
		glViewport(0, 0, textures[TEX_MAX_VER].width(), textures[TEX_MAX_VER].height());
		glBindTexture(GL_TEXTURE_2D, textures[TEX_MAX_HOR].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Draw output to screen */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, GScreenWidth, GScreenHeight);
		glUseProgram(programs[FS_COUNT].id());
		glBindTexture(GL_TEXTURE_2D, textures[((uint32_t *)t)[ARG_DRAWN_TEX]].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		eglSwapBuffers(GDisplay, GSurface);

		/* Read current time */
		clock_gettime(CLOCK_REALTIME, &gettime_now);
		time_difference = gettime_now.tv_nsec - start_time;
		if(time_difference < 0) time_difference += 1000000000;
		total_time_s += double(time_difference)/1000000000.0;
		start_time = gettime_now.tv_nsec;

		/* Print frame rate */
		float fr = float(double(cnt + 1) / total_time_s);
		if((cnt++ % 30) == 0){
			std::cout << fr << std::endl;
		}
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
