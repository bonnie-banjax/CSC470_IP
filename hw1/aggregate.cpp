
// ORD

#define CLIP(val, low, high) ((val) < (low) ? (low) : (val) > (high) ? (high) : (val))

// END

#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_clip:
//
// Clip intensities of image I1 to [t1,t2] range. Output is in I2.
// If    input<t1: output = t1;
// If t1<input<t2: output = input;
// If      val>t2: output = t2;
//
void
HW_clip(ImagePtr I1, int t1, int t2, ImagePtr I2)
{

// ORD

int lut[MXGRAY];
for(int i = 0; i < MXGRAY; i++) {
    if      (i < t1) lut[i] = t1;
    else if (i > t2) lut[i] = t2;
    else             lut[i] = i;
}

// END

}
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_contrast:
//
// Apply contrast enhancement to I1. Output is in I2.
// Stretch intensity difference from reference value (128) by multiplying
// difference by "contrast" and adding it back to 128. Shift result by
// adding "brightness" value.
//
void
HW_contrast(ImagePtr I1, double brightness, double contrast, ImagePtr I2)
{

// ORD
int lut[MXGRAY];
for(int i = 0; i < MXGRAY; i++) {
    double val = (i - 128.0) * contrast + 128.0 + brightness;
    lut[i] = CLIP(val, 0, MaxGray); // CLIP macro handles 0-255 bounds
}
// END

}
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{

// ORD
int lut[MXGRAY];
for(int i = 0; i < MXGRAY; i++) {
    double val = 255.0 * pow((double)i / 255.0, 1.0 / gamma);
    lut[i] = CLIP(val, 0, MaxGray);
}
// END

}
#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
//
void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if(approxAlg) {
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

}

void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{

// PUT YOUR CODE HERE

}
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{

// ORD
int lut[MXGRAY];
for(int i = 0; i < MXGRAY; i++) {
    if      (i <= t1) lut[i] = 0;
    else if (i >= t2) lut[i] = MaxGray;
    else              lut[i] = (int)(((i - t1) * 255.0) / (t2 - t1) + 0.5);
}
// END
}
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{

// ORD
int lut[MXGRAY];
int step = MaxGray / (levels - 1);
for(int i = 0; i < MXGRAY; i++) {
    int levelIdx = (i * levels) / MXGRAY;
    lut[i] = CLIP(levelIdx * step, 0, MaxGray);
}
// END

}
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_threshold:
//
// Threshold I1 using threshold thr. Output is in I2.
// input<thr: output=0;	 input >= thr: output=MaxGray (255)
//
void
HW_threshold(ImagePtr I1, int thr, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
	for(i=0; i<thr && i<MXGRAY; ++i) lut[i] = 0;
	for(   ;          i<MXGRAY; ++i) lut[i] = MaxGray;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// Note: IP_getChannel(I, ch, p1, type) gets pointer p1 of channel ch in image I.
	// The pixel datatype (e.g., uchar, short, ...) of that channel is returned in type.
	// It is ignored here since we assume that our input images consist exclusively of uchars.
	// IP_getChannel() returns 1 when channel ch exists, 0 otherwise.

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		for(i=0; i<total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}
