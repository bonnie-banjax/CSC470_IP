#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//
void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{
	if(I1 == I2) {
		ImagePtr source;
		IP_copyImage(I1, source);
		I1 = source;
	}
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	int kw = Ikernel->width();
	int kh = Ikernel->height();
	if(w <= 0 || h <= 0 || kw <= 0 || kh <= 0) return;

	// The GUI loads filter coefficients as a single floating-point channel.
	ChannelPtr<float> kernel;
	int type;
	if(!IP_getChannel(Ikernel, 0, kernel, type)) return;
	int centerX = kw / 2;
	int centerY = kh / 2;

	ChannelPtr<uchar> p1, p2;
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for(int y=0; y<h; y++) {
			for(int x=0; x<w; x++) {
				double sum = 0;
				for(int ky=0; ky<kh; ky++) {
					// Subtract the kernel offset to perform convolution,
					// including the kernel flip. Replicate border pixels.
					int yy = CLIP(y + centerY - ky, 0, h - 1);
					for(int kx=0; kx<kw; kx++) {
						int xx = CLIP(x + centerX - kx, 0, w - 1);
						sum += p1[yy*w + xx] * (double) kernel[ky*kw + kx];
					}
				}
				// Preserve supplied weights; do not normalize arbitrary kernels.
				sum = CLIP(sum, 0, MaxGray);
				p2[y*w + x] = ROUND(sum);
			}
		}
	}
}
