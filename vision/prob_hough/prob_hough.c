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
#include <time.h>
#include "prob_hough.h"

#if TRIG_LOOKUP
 #include "trig.h"
#else
 #define DEG2RAD 0.017453293f
#endif

#define PI 3.1416f

int phough_transform(img_t *img, phough_t *hs)
{
	srand (time(NULL));
	line_t p = {0} ;

	int width = img->w;
	int height = img->h;

	unsigned int numangle = hs->theta;
	unsigned int numrho = round((width + height) * 2 + 1);

	lines_t nzloc = {0};
	init_lines(&nzloc);

	for(p.y1 = 0; p.y1 < height; p.y1++) {
		for(p.x1 = 0; p.x1 < width; p.x1++ ){
			if(img->d[p.y1*width + p.x1]){
				img->d[p.y1*width + p.x1] = 1;
//				if(p.x1>3)
//					img->d[p.y1*width + p.x1 - 1] = 1;
					//img->d[p.y1*width + p.x1 - 2] = 1;
//					img->d[p.y1*width + p.x1 - 3] = 1;
				pushback_lines(&nzloc, &p);
			}
		}
	}

	int count = nzloc.count;
	pointer2_lines(&nzloc);

	#if DEBUG
		printf("numangle: %d, numrho: %d, count: %d\n", numangle, numrho, count);
	#endif
	line_t *tmp;
	int costs = 0;
	for(/**/; count > 0; count--)
	{
		// choose random point out of the remaining ones
		int idx = uniform_distribution(0, count);
		int max_val = hs->threshold-1, max_n = 0;

		line_t dline;
		float a, b;
		int k, x0, y0, dx0, dy0, xflag, n;

		int *adata = hs->accu;
		int i = nzloc.lp[idx]->y1, j = nzloc.lp[idx]->x1;
//		int i = hs->l->lp[idx]->y1, j = hs->l->lp[idx]->x1;

		int good_line;
		const int shift = 16;

		/*
		if(costs<5){
			printf("%X ", nzloc.lp[nzloc.count-1]);
			printf("%X ", nzloc.lp[nzloc.count-2]);
			printf("%X ", nzloc.lp[nzloc.count-3]);
			printf("%X ", nzloc.lp[nzloc.count-4]);
			printf("%X \n", nzloc.lp[nzloc.count-5]);
			costs++;
		}
		*/

		#if 1
		//	printf("Before: %X, ", nzloc.lp[idx]);
			tmp = nzloc.lp[idx];
			nzloc.lp[idx] = nzloc.lp[count-1];
			nzloc.lp[count-1] = tmp;
		//	printf("After: %X, %d\n", nzloc.lp[idx], idx);

		#else
			nzloc.l[idx].x1 = nzloc.l[count-1].x1;
			nzloc.l[idx].y1 = nzloc.l[count-1].y1;
		#endif
		
		if(!img->d[i*width + j]){
		    continue;
		}

		#if DEBUG
			printf("idx: %d, RMAX: %d ", idx, RAND_MAX);
			printf("Count: %d, ", count);
			printf("Promising, x: %d, y: %d\n", j, i);
		#endif

		// update accumulator, find the most probable line
		for( n = 0; n < numangle; n++, adata += numrho){
			int r = round(j*cos_table[n] + i*sin_table[n]);
			r += (numrho - 1) / 2;

			int val = ++adata[r];
			if( max_val < val ){
				#if DEBUG
					printf("n: %d, maxval: %d, numangle: %d\n", n, max_val, numangle);
				#endif
				max_val = val;
				max_n = n;
			}
		}

		// if it is too "weak" candidate, continue with another point
		if( max_val < hs->threshold )
		    continue;

		// from the current point walk in each direction
		// along the found line and extract the line segment
		a = sin_table[max_n]*-1;
		b = cos_table[max_n];
		x0 = j;
		y0 = i;

		//printf("%d", max_n);
		if( fabs(a) > fabs(b) ) //More horizontal line
		{
			xflag = 1;
			dx0 = a > 0 ? 1 : -1;
			dy0 = round( b*(1 << shift)/fabs(a) );
			y0 = (y0 << shift) + (1 << (shift-1));
		}else{			//More vertical
			xflag = 0;
			dy0 = b > 0 ? 1 : -1;
			dx0 = round( a*(1 << shift)/fabs(b) );
			x0 = (x0 << shift) + (1 << (shift-1));
		}

		for( k = 0; k < 2; k++ ){
			int gap = 0, x = x0, y = y0, dx = dx0, dy = dy0;

			if( k > 0 )
				dx = -dx, dy = -dy;

			for(;; x += dx, y += dy ){
				unsigned char* mdata;
				int i1, j1;

				if( xflag ){
					j1 = x;
					i1 = y >> shift;
				}else{
					j1 = x >> shift;
					i1 = y;
				}

				if( j1 < 0 || j1 >= width || i1 < 0 || i1 >= height )
					break;

				if( img->d[i1*width + j1] ){
					gap = 0;
					((int*)(&dline))[2*k+1] = i1;
					((int*)(&dline))[2*k] = j1;
				}else if( ++gap > hs->line_gap )
					break;
			}
		}

		good_line = abs(dline.x2 - dline.x1) >= hs->line_length ||
			    abs(dline.y2 - dline.y1) >= hs->line_length;

		for( k = 0; k < 2; k++ ){
			int x = x0, y = y0, dx = dx0, dy = dy0;

			if( k > 0 )
				dx = -dx, dy = -dy;

			for( ;; x += dx, y += dy )
			{
				int i1, j1;

				if( xflag ){
					j1 = x;
					i1 = y >> shift;
				}else{
					j1 = x >> shift;
					i1 = y;
				}

				if( img->d[i1*width + j1] ){
					if( good_line ){
						adata = (int*)hs->accu;
						for(n = 0; n < numangle; n++, adata += numrho ){
							int r = round( j1 * cos_table[n] + i1 * sin_table[n] );
							r += (numrho - 1) / 2;
							adata[r]--;
						}
					}
					img->d[i1*width + j1] = 0;
				}

				if(((int*)(&dline))[2*k+1] == i1 && ((int*)(&dline))[2*k] == j1)
					break;
			}
		}
		if( good_line ){
			pushback_lines(hs->l, &dline);
		}
	}
	clear_lines(&nzloc);
	free(nzloc.lp);
	return 0;
}

inline void init_hough(phough_t *hs)
{
	int i;

	hs->img_w = IMG_W;
	hs->img_h = IMG_H;
	hs->h = (int)(IMG_W*1.4142);
	hs->w = 180;
	hs->max = 0;

	hs->rho = 1;
	hs->threshold = 50;
	hs->theta = 180;
	hs->line_length = 50;
	hs->line_gap = 5;
	hs->line_max = 2000;


	memset(hs->accu, 0, (((IMG_W + IMG_H) * 2 + 1)*180)*sizeof(int));

	if(hs->l->l){				/* Clear the lines too */
		clear_lines(hs->l);
	}
	init_lines(hs->l);			/* Allocate memory for the new lines */
}

inline void init_lines(lines_t *v)
{
	v->size = LINES_INITIAL;
	v->count = 0;

	v->l = (line_t*) calloc(v->size, sizeof(line_t));

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

inline void pointer2_lines(lines_t *v)
{
	int i;
	line_t *t = v->l;

	v->lp = (line_t**)calloc(v->count, sizeof(line_t*));

	if(!v->lp){
		printf("PANIC\n");
	}

	#if DEBUG
		printf("Count: %d\n", v->count);
	#endif

	for(i = 0; i < v->count; i++){
		#if DEBUG
			if(i < 5)
				printf("Add: %X",t);
		#endif

		v->lp[i] = t++;		
	}
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
	#if DEBUG >= 2
		printf("x1: %d, y1: %d, x2: %d, y2: %d, s: %d\n", new->x1, new->y1, new->x2, new->y2, new->s);
	#endif
	if(v->count == v->size){
		line_t *temp;
		v->size *= LINES_FACTOR;
		temp = (line_t*)realloc(v->l, v->size * sizeof(line_t));
		if(temp) {
			v->l = temp;				
		}else{
			printf("PANIC OUT");
		}
		#if DEBUG
			printf("Resized to: %d\n", v->size);
		#endif	
	}
	memcpy ( v->l + v->count, new, sizeof(line_t));
	(v->count)++;
}

/*
	http://stackoverflow.com/questions/11641629/generating-a-uniform-distribution-of-integers-in-c
 */
int uniform_distribution(int rangeLow, int rangeHigh)
{

	double myRand = rand()/(1.0 + RAND_MAX); 
	int range = rangeHigh - rangeLow + 1;
	int myRand_scaled = (myRand * range) + rangeLow;
 return myRand_scaled;
}
