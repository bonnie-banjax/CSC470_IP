#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//
// The input is padded by pixel replication. A circular buffer of sz padded
// rows holds the rows needed for the current output scanline. For each
// output pixel, the sz x sz neighborhood is sorted and the middle value
// (the median) is written to the output.
//
void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width and height
	int w = I1->width ();
	int h = I1->height();

	// clamp sz to [1,9] and force it to be odd
	if(sz < 1) sz = 1;
	if(sz > 9) sz = 9;
	if(sz % 2 == 0) sz++;

	int pad = sz / 2;			// padding on each side
	int bw  = w + 2*pad;			// width of a padded row

	// circular buffer of sz padded rows, and storage for one neighborhood
	vector<uchar> buf(sz * bw);
	vector<uchar> nbr(sz * sz);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

		// copy padded row k (k = 0 .. h+2*pad-1) into circular buffer slot k%sz.
		// Padded row k is input row k-pad, clamped to [0,h-1] (pixel replication).
		auto copyRow = [&](int k) {
			int    y   = CLIP(k - pad, 0, h-1);
			uchar *in  = &p1[y*w];
			uchar *row = &buf[(k % sz) * bw];
			for(int x=0; x<pad; x++) {
				row[x]           = in[0];
				row[pad + w + x] = in[w-1];
			}
			for(int x=0; x<w; x++) row[pad + x] = in[x];
		};

		// init circular buffer with the first sz padded rows
		for(int k=0; k<sz; k++) copyRow(k);

		for(int y=0; y<h; y++) {
			// padded rows y .. y+sz-1 are in the buffer; center row is y+pad
			for(int x=0; x<w; x++) {
				// gather sz x sz neighborhood centered at (x, y)
				int n = 0;
				for(int j=0; j<sz; j++) {
					uchar *row = &buf[((y + j) % sz) * bw];
					for(int i=0; i<sz; i++) nbr[n++] = row[x + i];
				}

				// sort neighborhood and output the middle value
				std::sort(nbr.begin(), nbr.end());
				*p2++ = nbr[n/2];
			}

			// replace oldest row (y) with next padded row (y+sz)
			if(y+1 < h) copyRow(y + sz);
		}
	}
}
