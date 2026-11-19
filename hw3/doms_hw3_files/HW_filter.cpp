#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_filter:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_filter:
//
// Convolve a box filter of size (sx, sy) using FFT.
// Convolution in the spatial domain is multiplication in frequency domain.
// straddle = true , split box filter to four segments and put at corners
// straddle = false, put box filter in the center
//
// With straddle, the box is centered on pixel (0,0) and wraps around to the
// four corners, so the blurred output stays in place. Centered in the image,
// the box is shifted by (w/2, h/2), so the output is circularly shifted too.
//
void
HW_filter(ImagePtr I1, int sx, int sy, bool straddle, ImagePtr I2)
{
	int w = I1->width();
	int h = I1->height();

	// allocate a kernel image the same size as input image
	ImagePtr Ikernel = IP_allocImage(w, h, FLOATCH_TYPE);

	// clear kernel image
	IP_clearImage(Ikernel);

	// init pointer to kernel image
	ChannelPtr<float> p = Ikernel[0];

	// normalize box filter values by scaling to total size of input image
	float norm = (float(w*h)) / (sx*sy);

	// box extends from -sx/2 to sx-1-sx/2 horizontally (same for y)
	int x0 = -(sx/2), y0 = -(sy/2);

	// init kernel image with normalized box filter values
	if(straddle) {		// straddle four corners
		for(int y=y0; y<y0+sy; y++)
			for(int x=x0; x<x0+sx; x++)
				p[((y + h) % h)*w + (x + w) % w] = norm;	// wrap around
	} else {		// center box filter in center
		for(int y=y0; y<y0+sy; y++)
			for(int x=x0; x<x0+sx; x++)
				p[(h/2 + y)*w + (w/2 + x)] = norm;
	}

	ImagePtr Ifft1 = NEWIMAGE, Ifft2 = NEWIMAGE, Ifft3 = NEWIMAGE, IinvFFT3 = NEWIMAGE;
	ImagePtr Imag = NEWIMAGE, Iphase = NEWIMAGE;

	// convolve input image and kernel by multiplying their FFTs in the frequency domain
	IP_fft2D(I1, 1, Ifft1);			// compute FFT of input image
	IP_fft2D(Ikernel, 1, Ifft2);		// compute FFT of kernel
	IP_multiplyCmplx(Ifft1, Ifft2, Ifft3);	// complex multiplication of the two FFTs

	// compute the magnitude spectrum of the inverse FFT
	IP_fft2D(Ifft3, -1, IinvFFT3);		// compute inverse FFT of Ifft3
	HW_fft2MagPhase(IinvFFT3, Imag, Iphase);	// get magnitude spectrum from IinvFFT3

	// allocate output image and save magnitude spectrum for display
	I2->allocImage(Imag->width(), Imag->height(), BW_TYPE);
	I2->setImageType(BW_IMAGE);
	IP_castChannel(Imag, 0, I2, 0, UCHAR_TYPE);
}
