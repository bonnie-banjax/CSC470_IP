#include "IP.h"
using namespace IP;

extern void HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
// Unsharp masking: blur I1 with a size x size box filter (HW_blur),
// subtract the blurred image from I1 to get the edges, scale the
// difference by factor, and add it back to I1:
//	output = I1 + factor * (I1 - blur(I1)), clipped to [0,255]
//
void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// blur input image with a size x size box filter
	ImagePtr Iblur = NEWIMAGE;
	HW_blur(I1, size, size, Iblur);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, pb;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2,    ch, p2, type);		// get output pointer for channel ch
		IP_getChannel(Iblur, ch, pb, type);		// get blur   pointer for channel ch

		for(int i=0; i<total; i++) {
			// add scaled difference (edges) back to input pixel
			double val = *p1 + factor * (*p1 - *pb);
			int    v   = ROUND(val);
			*p2++ = CLIP(v, 0, MaxGray);		// clip to [0,255]
			p1++;
			pb++;
		}
	}
}
