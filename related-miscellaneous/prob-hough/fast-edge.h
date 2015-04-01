/*
	FAST-EDGE
	Copyright (c) 2009 Benjamin C. Haynor

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _FASTEDGE
#define _FASTEDGE
#define LOW_THRESHOLD_PERCENTAGE 0.8 // percentage of the high threshold value that the low threshold shall be set at
#define PI 3.14159265
#define HIGH_THRESHOLD_PERCENTAGE 0.10 // percentage of pixels that meet the high threshold - for example 0.15 will ensure that at least 15% of edge pixels are considered to meet the high threshold

#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) < (Y) ? (Y) : (X))

#include "buildflags.h"

//#define CLOCK			// uncomment to show running times of image processing functions (in seconds)
//#define ABS_APPROX		// uncomment to use the absolute value approximation of sqrt(Gx ^ 2 + Gy ^2)
//#define PRINT_HISTOGRAM	// uncomment to print the histogram used to estimate the threshold

void canny_edge_detect(struct img * img_in, struct img * img_out);
void gaussian_noise_reduce(struct img * img_in, struct img * img_out);
void calc_gradient_sobel(struct img * img_in, int g[], int dir[]);
void calc_gradient_scharr(struct img * img_in, int g_x[], int g_y[], int g[], int dir[]);
void non_max_suppression(struct img * img, int g[], int dir[]);
void estimate_threshold(struct img * img, int * high, int * low);
void hysteresis (int high, int low, struct img * img_in, struct img * img_out);
int trace (int x, int y, int low, struct img * img_in, struct img * img_out);
int range (struct img * img, int x, int y);
void dilate_1d_h(struct img * img, struct img * img_out);
void dilate_1d_v(struct img * img, struct img * img_out);
void erode_1d_h(struct img * img, struct img * img_out);
void erode_1d_v(struct img * img, struct img * img_out);
void erode(struct img * img_in, struct img * img_scratch, struct img * img_out);
void dilate(struct img * img_in, struct img * img_scratch, struct img * img_out);
void morph_open(struct img * img_in, struct img * img_scratch, struct img * img_scratch2, struct img * img_out);
void morph_close(struct img * img_in, struct img * img_scratch, struct img * img_scratch2, struct img * img_out);
#endif
