#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//
void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
	if(I1 == I2) {
		ImagePtr source;
		IP_copyImage(I1, source);
		I1 = source;
	}
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	if(w <= 0 || h <= 0) return;

	size = MAX(size, 1);
	if(size % 2 == 0) size++;
	int radius = size / 2;
	double neighborWeight = -factor / ((double) size * size);

	ChannelPtr<uchar> p1, p2;
	int type;
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for(int y=0; y<h; y++) {
			for(int x=0; x<w; x++) {
				long long sum = 0;
				for(int dy=-radius; dy<=radius; dy++) {
					int yy = CLIP(y + dy, 0, h - 1);
					for(int dx=-radius; dx<=radius; dx++) {
						int xx = CLIP(x + dx, 0, w - 1);
						sum += p1[yy*w + xx];
					}
				}
				// Combined kernel: -factor/area everywhere, plus
				// (1 + factor) at the center. No intermediate blur.
				double value = (1 + factor) * p1[y*w + x]
				             + neighborWeight * sum;
				value = CLIP(value, 0, MaxGray);
				p2[y*w + x] = ROUND(value);
			}
		}
	}
}
