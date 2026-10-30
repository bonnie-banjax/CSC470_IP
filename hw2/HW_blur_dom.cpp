#include "IP.h"
using namespace IP;

static void blur1D(uchar *buf, int len, int ww, uchar *out, int stride);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
// Separable implementation: first blur each row of I1 into a temporary
// image, then blur each column of the temporary image into I2.
// Scanlines are copied into a buffer padded by pixel replication so the
// window never falls off the edge of the image.
//
void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width and height
	int w = I1->width ();
	int h = I1->height();

	// force kernel dimensions to be odd
	if(filterW < 1) filterW = 1;
	if(filterH < 1) filterH = 1;
	if(filterW % 2 == 0) filterW++;
	if(filterH % 2 == 0) filterH++;

	// temporary image for results of the horizontal (row) pass
	ImagePtr Itmp = NEWIMAGE;
	IP_copyImageHeader(I1, Itmp);

	// buffer large enough to hold a padded row or a padded column
	int padW = filterW / 2;
	int padH = filterH / 2;
	int bufLen = MAX(w + 2*padW, h + 2*padH);
	uchar *buf = new uchar[bufLen];

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, pt;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2,   ch, p2, type);		// get output pointer for channel ch
		IP_getChannel(Itmp, ch, pt, type);		// get temp   pointer for channel ch

		// pass 1: blur rows of I1 into Itmp
		for(int y=0; y<h; y++) {
			uchar *in = &p1[y*w];

			// copy row into buffer, padding both ends by pixel replication
			for(int x=0; x<padW; x++) {
				buf[x]              = in[0];
				buf[padW + w + x]   = in[w-1];
			}
			for(int x=0; x<w; x++) buf[padW + x] = in[x];

			blur1D(buf, w, filterW, &pt[y*w], 1);
		}

		// pass 2: blur columns of Itmp into I2
		for(int x=0; x<w; x++) {
			// copy column into buffer, padding both ends by pixel replication
			for(int y=0; y<padH; y++) {
				buf[y]              = pt[x];
				buf[padH + h + y]   = pt[(h-1)*w + x];
			}
			for(int y=0; y<h; y++) buf[padH + y] = pt[y*w + x];

			blur1D(buf, h, filterH, &p2[x], w);
		}
	}

	delete [] buf;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blur1D:
//
// Blur a padded scanline buf with a box filter of (odd) width ww.
// buf holds len pixels plus ww/2 padded pixels on each end.
// The len output pixels are written to out, spaced stride apart.
// A running sum adds the incoming pixel and subtracts the outgoing pixel
// as the window slides along the scanline; each output is sum/ww.
//
static void
blur1D(uchar *buf, int len, int ww, uchar *out, int stride)
{
	// init sum with the window centered on the first pixel
	int sum = 0;
	for(int i=0; i<ww; i++) sum += buf[i];

	for(int i=0; i<len; i++) {
		*out = sum / ww;			// average of window
		out += stride;
		if(i < len-1)
			sum += buf[i+ww] - buf[i];	// add incoming, subtract outgoing
	}
}
