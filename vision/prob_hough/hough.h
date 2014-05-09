#ifndef HOUGH_H_
#define HOUGH_H_

#include "buildflags.h"
#if SDL_USED
 #include <SDL/SDL.h>
 #include <SDL/SDL_gfxPrimitives.h>
#endif

typedef struct point {
	int x;
	int y;
} point_t;

typedef struct line {
	int x1;
	int y1;
	int x2;
	int y2;
	int s;
} line_t;

/*
 struct lines: Container of "line" structures.

	count: Number of stored lines.
	size: Number of current slots (duplicated when full)
 */
typedef struct lines {
	unsigned long int count; 
	unsigned long int size;
	line_t *l;
} lines_t;

typedef struct hough {
	int img_w;
	int img_h;
	int w;
	int h;
	int max;
	unsigned int *accu;
	lines_t * l;
} hough_t;

typedef struct roi {
	int x1;
	int y1;
	int x2;
	int y2;
} roi_t;

typedef struct img {
	int w;
	int h;
	unsigned char * d;
	roi_t *r;
} img_t;

int hough_transform(img_t *img, hough_t *hs);
int hough_getlines(int threshold, hough_t *hs);
inline void init_hough(hough_t *hs);

inline void init_lines(lines_t *v);
inline void pushback_lines(lines_t* v, line_t* new);
inline void clear_lines(lines_t *v);


#if SDL_USED
 inline Uint32 get_pixel32( SDL_Surface *surface, int x, int y );
 inline void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel );
 inline void wait();
#endif

#endif /* HOUGH_H_ */
