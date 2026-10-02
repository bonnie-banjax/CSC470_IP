#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
// input<=t1: output=0;  input>=t2: output=MaxGray (255)
// t1<input<t2: output = MaxGray * (input-t1) / (t2-t1)
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
	for(i=0; i<MXGRAY; ++i) {
		if(t2 <= t1)	lut[i] = (i < t1) ? 0 : MaxGray;	// no range to stretch: threshold at t1
		else if(i <= t1) lut[i] = 0;				// pull values below t1 to black
		else if(i >= t2) lut[i] = MaxGray;			// pull values above t2 to white
		else		lut[i] = ROUND((double) MaxGray * (i - t1) / (t2 - t1));
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		for(i=0; i<total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}
