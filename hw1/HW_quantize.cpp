#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
// The range [0,255] is split into levels uniform intervals of width
// scale = 256/levels. Each input maps to the midpoint of its interval:
// output = scale * (int) (input/scale) + bias, where bias = 128/levels.
// For levels=4, the output gray values are 32, 96, 160, and 224.
// If dither is set, a random jitter in [0,bias] is alternately added to and
// subtracted from successive pixels prior to quantization.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// width of each quantization interval, and offset to its midpoint
	double scale = (double) MXGRAY / levels;
	double bias  = 128.0 / levels;

	// init lookup table: map each gray level to midpoint of its interval
	int i, lut[MXGRAY];
	for(i=0; i<MXGRAY; ++i)
		lut[i] = CLIP(ROUND(scale * (int) (i/scale) + bias), 0, MaxGray);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		if(!dither) {
			for(i=0; i<total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
		} else {
			int sign = 1;					// alternates between +1 and -1
			for(i=0; i<total; i++) {
				// random jitter in [0,bias]
				double jitter = ((double) rand() / RAND_MAX) * bias;

				// add or subtract jitter to input pixel, then clip to [0,MaxGray]
				// (read pixel first: ROUND is a macro that evaluates its argument twice)
				double val = *p1++ + sign*jitter;
				int v = ROUND(val);
				v = CLIP(v, 0, MaxGray);
				sign = -sign;				// switch sign for next pixel

				*p2++ = lut[v];				// use lut[] to eval output
			}
		}
	}
}
