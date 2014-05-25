/*
	(Ab)using preprocessor conditionals wherever it is possible,
	produces faster code with less branches. Recompilation is the trade-off.
 */

#ifndef BUILDFLAGS_H_
#define BUILDFLAGS_H_

 #define DEBUG		0
 #define SANITY_CHECKS	0
 #define STATIC_ALLOC	1
 #define PRINT_TIME	1
 #define TRIG_LOOKUP	1

 /* These are constants and should probably be defined as such instead */
 #define IMG_W		640
 #define IMG_H		480
 #define LINES_FACTOR	3
 #define LINES_INITIAL	20

 #define ITERATIONS	1

 #define WAIT_KEY	0
 #define DRAW_ACCUM	0
 #define DRAW_OUTPUT	0
 #define LOAD_BMP_SDL	1
 #define SDL_USED	DRAW_ACCUM || DRAW_OUTPUT || LOAD_BMP_SDL

 #define ARGB_R 0x00ff0000
 #define ARGB_G 0x0000ff00
 #define ARGB_B 0x000000ff

#endif
