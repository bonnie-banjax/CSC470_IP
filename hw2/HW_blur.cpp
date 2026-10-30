#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// Preserve the input if the caller requests an in-place operation.
	if(I1 == I2) {
		ImagePtr source;
		IP_copyImage(I1, source);
		I1 = source;
	}
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	if(w <= 0 || h <= 0) return;

	// A box kernel has equal weights, each 1 / (filterW * filterH).
	filterW = MAX(filterW, 1);
	filterH = MAX(filterH, 1);
	if(filterW % 2 == 0) filterW++;
	if(filterH % 2 == 0) filterH++;
	int radiusX = filterW / 2;
	int radiusY = filterH / 2;
	double area = (double) filterW * filterH;

	ChannelPtr<uchar> p1, p2;
	int type;
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for(int y=0; y<h; y++) {
			for(int x=0; x<w; x++) {
				long long sum = 0;
				for(int dy=-radiusY; dy<=radiusY; dy++) {
					int yy = CLIP(y + dy, 0, h - 1);
					for(int dx=-radiusX; dx<=radiusX; dx++) {
						// Replicate the nearest pixel at image boundaries.
						int xx = CLIP(x + dx, 0, w - 1);
						sum += p1[yy*w + xx];
					}
				}
				p2[y*w + x] = ROUND(sum / area);
			}
		}
	}
}
