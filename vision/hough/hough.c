// **********************************************************************************
//
// BSD License.
// This file is part of a Hough Transformation tutorial,
// see: http://www.keymolen.com/2013/05/hough-transformation-c-implementation.html
//
// Copyright (c) 2013, Bruno Keymolen, email: bruno.keymolen@gmail.com
// All rights reserved.
//
// This code was ported to C and heavily modified by Pedro Aguiar if something
// does not work it is probably his fault :). paguiar32@gmail.com
//
// **********************************************************************************

#include "buildflags.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "hough.h"

#if TRIG_LOOKUP
 #include "trig.h"
#else
 #define DEG2RAD 0.017453293f
#endif

int hough_transform(img_t *img, hough_t *hs)
{
	int x, y, t;

	double hough_h = 1.4142f*(double)IMG_W/ 2.0f;
	double center_x = img->w/2;
	double center_y = img->h/2;

	for(y = img->r->y1; y < img->r->y2; y++)
	{
 		for(x = img->r->x1; x < img->r->x2; x++)
		{
			if(img->d[(y*img->w) + x] > 250)
			{
				for(t = 0; t < 180; t++)
				{
					#if TRIG_LOOKUP
						double r = (((double)x - center_x) * cos_table[t] + (((double)y - center_y) * sin_table[t]));
					#else
						double r = (((double)x - center_x) * cos((double)t * DEG2RAD)) + (((double)y - center_y) * sin((double)t * DEG2RAD));
					#endif

					if(++(hs->accu[(int)((round(r + hough_h) * 180.0)) + t]) > hs->max){
						hs->max++;
					}
				}
			}
		}
	}

	return 0;
}

int hough_getlines(int threshold, hough_t *hs)
{
	int r, t, ly, lx;

	#if SANITY_CHECKS
		if(hs->accu == 0)
			return -1;
	#endif

	for(r = 0; r < hs->h; r++)
	{
		for(t = 0; t < hs->w; t++)
		{
			if((int)hs->accu[(r*hs->w) + t] >= threshold)
			{
				//Is this point a local maxima (9x9)
				int max = hs->accu[(r * hs->w) + t];
				for(ly = -LOCALMAX; ly <= LOCALMAX; ly++)
				{
					for(lx = -LOCALMAX; lx <= LOCALMAX; lx++)
					{
						if( (ly+r >= 0 && ly + r < hs->h) && (lx + t >= 0 && lx + t < hs->w)  )
						{
							if( (int)hs->accu[((r + ly) * hs->w) + (t + lx)] > max )
							{
								max = hs->accu[((r + ly) * hs->w) + (t + lx)];
								ly = lx = LOCALMAX;
							}
						}
					}
				}
				if(max > (int)hs->accu[(r*hs->w) + t])
					continue;

				line_t L;
				L.s = (int)hs->accu[(r*hs->w) + t];
				if(t >= 45 && t <= 135)
				{
					//y = (r - x cos(t)) / sin(t)
					L.x1 = 0;
					L.x2 = hs->img_w;
					#if TRIG_LOOKUP
						L.y1 = ((double)(r-(hs->h/2)) - ((L.x1 - (hs->img_w/2) ) * cos_table[t])) / sin_table[t] + (hs->img_h / 2);
						L.y2 = ((double)(r-(hs->h/2)) - ((L.x2 - (hs->img_w/2) ) * cos_table[t])) / sin_table[t] + (hs->img_h / 2);
					#else
						L.y1 = ((double)(r-(hs->h/2)) - ((L.x1 - (hs->img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (hs->img_h / 2);
						L.y2 = ((double)(r-(hs->h/2)) - ((L.x2 - (hs->img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (hs->img_h / 2);
					#endif
				}
				else
				{
					//x = (r - y sin(t)) / cos(t);
					L.y1 = 0;
					L.y2 = hs->img_h;
					#if TRIG_LOOKUP
						L.x1 = ((double)(r-(hs->h/2)) - ((L.y1 - (hs->img_h/2) ) * sin_table[t])) / cos_table[t] + (hs->img_w / 2);
						L.x2 = ((double)(r-(hs->h/2)) - ((L.y2 - (hs->img_h/2) ) * sin_table[t])) / cos_table[t] + (hs->img_w / 2);
					#else
						L.x1 = ((double)(r-(hs->h/2)) - ((L.y1 - (hs->img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (hs->img_w / 2);
						L.x2 = ((double)(r-(hs->h/2)) - ((L.y2 - (hs->img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (hs->img_w / 2);
					#endif

				}

				pushback_lines(hs->l, &L);
			}
		}
	}

	return 0;
}

inline void init_hough(hough_t *hs)
{
	int i;

	hs->img_w = IMG_W;
	hs->img_h = IMG_H;
	hs->h = (int)(IMG_W*1.4142);
	hs->w = 180;
	hs->max = 0;

	for(i=0; i< (hs->w * hs->h); i++){ 	/* Clear the accumulator of previous usage */
		hs->accu[i] = 0;
	}

	if(hs->l){				/* Clear the lines too */
		clear_lines(hs->l);
	}
	init_lines(hs->l);			/* Allocate memory for the new lines */
}

inline void init_lines(lines_t *v)
{
	v->size = LINES_INITIAL;
	v->count = 0;
	v->l = (line_t*) calloc (v->size, sizeof(line_t));

	#if SANITY_CHECKS
		if(v->l){
			#if DEBUG
				printf(" Lines memory allocated properly\n");
			#endif
		}else{
			#if DEBUG
				printf(" Memory allocation error\n");
			#endif
		}
	#endif
}

inline void clear_lines(lines_t *v)
{
	if(v){
		#if DEBUG
			printf("size: %d, count: %d\n", v->size, v->count);
		#endif
		if(v->l){
			free(v->l);
			v->l = NULL;
		}
	}
}

inline void pushback_lines(lines_t* v, line_t* new)
{
	#if DEBUG
		printf("x1: %d, y1: %d, x2: %d, y2: %d, s: %d\n", new->x1, new->y1, new->x2, new->y2, new->s);
	#endif
	if(v->count == v->size){
		line_t *temp;
		v->size *= LINES_FACTOR;
		temp = (line_t*)realloc(v->l, v->size * sizeof(line_t));
		if(temp) {
			v->l = temp;				
		}
	}
	memcpy ( v->l + v->count, new, sizeof(line_t));
	(v->count)++;
}
