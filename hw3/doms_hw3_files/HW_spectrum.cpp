#include <cmath>
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Convolve magnitude and phase spectrum from image I1.
// Output is in Imag and Iphase.
//

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_fft2D(ImagePtr I1, int dir, ImagePtr Ifft);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Compute magnitude and phase spectrum from input image I1.
// Save results in Imag and Iphase.
// The 2-D FFT is our separable implementation (fft1D on rows, then columns).
// Magnitude and phase are each scaled from [min,max] to [0,255] for display.
// The magnitude is first compressed with log(1 + k*mag): the DC term is many
// orders of magnitude larger than the other frequencies, so a linear scaling
// would map nearly every other frequency to black. The forward FFT scales by
// 1/N, so most magnitudes are tiny; k = 16 lifts them into the range where
// the log takes effect (chosen to match the brightness of the sample app).
//
void
HW_spectrum(ImagePtr I1, ImagePtr Imag, ImagePtr Iphase)
{
	int w = I1->width();
	int h = I1->height();
	int total = w*h;

	// compute FFT of the input image
	ImagePtr Ifft = NEWIMAGE;
	HW_fft2D(I1, 1, Ifft);

	// compute magnitute and phase spectrums from FFT image
	ImagePtr Im = NEWIMAGE;
	ImagePtr Ip = NEWIMAGE;
	HW_fft2MagPhase(Ifft, Im, Ip);

	// compress the dynamic range of the magnitude spectrum for display
	const double k = 16;
	ChannelPtr<float> m = Im[0];
	for(int i=0; i<total; i++) m[i] = log(1.0 + k*m[i]);

	// find min and max of magnitude and phase spectrums
	double magMin, magMax, phaseMin, phaseMax;
	IP_minmaxChannel(Im, 0, magMin,   magMax  );
	IP_minmaxChannel(Ip, 0, phaseMin, phaseMax);

	// allocate uchar image for displaying magnitude and phase
	Imag  ->allocImage(w, h, BW_TYPE);
	Iphase->allocImage(w, h, BW_TYPE);

	// set imagetypes for single 8-bit channel
	Imag  ->setImageType(BW_IMAGE);
	Iphase->setImageType(BW_IMAGE);

	// get channel pointers for input magnitude and phase spectrums
	ChannelPtr<float>   magF = Im[0];
	ChannelPtr<float> phaseF = Ip[0];

	// get channel pointers for output magnitude and phase spectrums
	ChannelPtr<uchar> mag    = Imag  [0];
	ChannelPtr<uchar> phase  = Iphase[0];

	// scale magnitude and phase to fit between [0, 255]
	double magScale   = (magMax   > magMin  ) ? MaxGray / (magMax   - magMin  ) : 0;
	double phaseScale = (phaseMax > phaseMin) ? MaxGray / (phaseMax - phaseMin) : 0;
	for(int i=0; i<total; i++) {
		int m = ROUND((magF  [i] - magMin  ) * magScale  );
		int p = ROUND((phaseF[i] - phaseMin) * phaseScale);
		mag  [i] = CLIP(m, 0, MaxGray);
		phase[i] = CLIP(p, 0, MaxGray);
	}
}
