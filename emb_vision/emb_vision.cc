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
 uint8_t frame_data_l[IMG_SIZE];
 uint8_t frame_data_r[IMG_SIZE];
#else
 #include "img.h"
#endif

extern uint32_t GScreenWidth;
extern uint32_t GScreenHeight;
extern EGLDisplay GDisplay;
extern EGLSurface GSurface;
extern EGLContext GContext;

/* Prototypes */
void *Capture(void *t);
void Depth(void *t);
void *Lane(void *t);

/* Pointer to array of pointers to functions */
//void *(*f_thread[3])(void *t) = {Capture, Depth, Lane};

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
			case 'g':
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
#if 0
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
#endif
	Depth(NULL);
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

void Depth(void *t)
{
	std::vector<GLTexture> textures;
	std::vector<GLShader> shaders;
	std::vector<GLProgram> programs;

	GLint k_location;
	int left_index = 0;
	int right_index = 1;
	int stereo_index = 2;
	int smallest_index = 3;
	int dmap_index = 7;

	InitGraphics(textures, shaders, programs);

	/* Set offset and scale uniforms, get texelsize locations */
	for(std::vector<GLProgram>::size_type i = 0; i < programs.size(); i++) {
		glUseProgram(programs[i].id());
		glUniform2f(glGetUniformLocation(programs[i].id(), "offset"), -1.0f, -1.0f);
		glUniform2f(glGetUniformLocation(programs[i].id(), "scale"), 2.0f, 2.0f);
		programs[i].set_texel_location(glGetUniformLocation(programs[i].id(), "texelsize"));
	}

	/* Set constant texelsizes */
	glUseProgram(programs[FS_PACK].id());
	glUniform2f(programs[FS_PACK].id(), 1.0f/textures[left_index].width(), 1.0f/textures[left_index].height());
	glUseProgram(programs[FS_DMAP0].id());
	glUniform2f(programs[FS_DMAP0].texel_location(), 1.0f/textures[smallest_index].width(), 1.0f/textures[smallest_index].height());
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

	k_location = glGetUniformLocation(programs[FS_CONTRAST].id(), "k");

	glBindTexture(GL_TEXTURE_2D, textures[stereo_index].id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);

#if 0
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
#else
	textures[TEX_INLEFT].SetPixels(frame_data_l);
	textures[TEX_INRIGHT].SetPixels(frame_data_r);

#endif
	while(run){
#if 0
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

		textures[TEX_INLEFT].SetPixels(frame_data_l);
		textures[TEX_INRIGHT].SetPixels(frame_data_r);

		/* Let other threads know this thread is done READING current frame */
		pthread_mutex_lock(&frame_mutex);
		frame--;
		#if DEBUG
			printf("\tDepth done...\n");
		#endif
		pthread_mutex_unlock(&frame_mutex);
#endif
		/* Clear the background */
		// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		/* Capture POT region and pack to a single texture */
		glUseProgram(programs[FS_PACK].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[stereo_index].framebuffer_id());
		glViewport(0, 0, textures[stereo_index].width(), textures[stereo_index].height());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[left_index].id());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[right_index].id());
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Generate the mipmaps */
		glBindTexture(GL_TEXTURE_2D, textures[stereo_index].id());
		glGenerateMipmap(GL_TEXTURE_2D);

		/* Lowest resolution pass */
		glUseProgram(programs[FS_DMAP0].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[smallest_index].framebuffer_id());
		glViewport(0, 0, textures[smallest_index].width(), textures[smallest_index].height());
		glBindTexture(GL_TEXTURE_2D, textures[stereo_index].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Grow the previous disparity map, calculate new disparities, loop */
		for(int level = 0; level < 4; level++){
			/* Grow the previous */
			int grow_index = smallest_index + level;
			int grown_index = grow_index + 1;
			glUseProgram(programs[FS_DINTERPOLATE].id());
			glUniform2f(programs[FS_DINTERPOLATE].texel_location(), 1.0f/textures[grow_index].width(), 1.0f/textures[grow_index].height());
			glBindFramebuffer(GL_FRAMEBUFFER, textures[grown_index].framebuffer_id());
			glViewport(0, 0, textures[grown_index].width(), textures[grown_index].height());
			glBindTexture(GL_TEXTURE_2D, textures[grow_index].id());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			/* Calculate new disparities */
			glUseProgram(programs[FS_DMAPN].id());
			glUniform2f(programs[FS_DMAPN].texel_location(), 1.0f/textures[grown_index].width(), 1.0f/textures[grown_index].height());
			glBindFramebuffer(GL_FRAMEBUFFER, textures[grown_index].framebuffer_id());
			glViewport(0, 0, textures[grown_index].width(), textures[grown_index].height());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[stereo_index].id());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[grown_index].id());
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		/* Increase contrast for visualization */
		glUseProgram(programs[FS_CONTRAST].id());
		for(int level = 0; level <= 4; level++){
			int contrast_index = smallest_index + level;
			glUniform1f(k_location, 6*(4-level+1));
			glUniform2f(programs[FS_CONTRAST].texel_location(), 1.0f/textures[contrast_index].width(), 1.0f/textures[contrast_index].height());
			//runShader(gl, textures[contrast_index], textures[contrast_index]);
		}

		/* Filter to remove noise from inaccurate match */
		glUseProgram(programs[FS_DILATE].id());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[dmap_index + 1].framebuffer_id());
		glViewport(0, 0, textures[dmap_index + 1].width(), textures[dmap_index + 1].height());
		glBindTexture(GL_TEXTURE_2D, textures[dmap_index].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Calculate the maximum by row of 8 elements */
		glUseProgram(programs[FS_ERODE].id());
		glUniform2f(programs[FS_ERODE].texel_location(), 1.0f/textures[dmap_index + 1].width(), 0.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, textures[dmap_index + 2].framebuffer_id());
		glViewport(0, 0, textures[dmap_index + 2].width(), textures[dmap_index + 2].height());
		glBindTexture(GL_TEXTURE_2D, textures[dmap_index + 1].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Calculate the maximum by column of 8 elements */
		glUniform2f(programs[FS_ERODE].texel_location(), 0.0f, 1.0f/textures[dmap_index + 2].height());
		glBindFramebuffer(GL_FRAMEBUFFER, textures[dmap_index + 3].framebuffer_id());
		glViewport(0, 0, textures[dmap_index + 3].width(), textures[dmap_index + 3].height());
		glBindTexture(GL_TEXTURE_2D, textures[dmap_index + 2].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Draw output to screen */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, GScreenWidth, GScreenHeight);
		glUseProgram(programs[FS_COUNT].id());
		glBindTexture(GL_TEXTURE_2D, textures[TEX_INLEFT].id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		eglSwapBuffers(GDisplay,GSurface);
	}
#if 0
 pthread_exit(NULL);
#endif
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
