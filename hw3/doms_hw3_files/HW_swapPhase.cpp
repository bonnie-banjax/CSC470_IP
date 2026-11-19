#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft);
extern void HW_fft2D(ImagePtr I1, int dir, ImagePtr Ifft);

static void magToUchar(ImagePtr Ifft, ImagePtr Iout);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
	ImagePtr Ifft1 = NEWIMAGE, Ifft2 = NEWIMAGE, IinvFFT1 = NEWIMAGE, IinvFFT2 = NEWIMAGE;
	ImagePtr Imag1 = NEWIMAGE, Iphase1 = NEWIMAGE, Imag2 = NEWIMAGE, Iphase2 = NEWIMAGE;

	// compute FFT of I1 and I2
	HW_fft2D(I1, 1, Ifft1);
	HW_fft2D(I2, 1, Ifft2);

	// compute magnitude and phase from real and imaginary FFT channels
	HW_fft2MagPhase(Ifft1, Imag1, Iphase1);
	HW_fft2MagPhase(Ifft2, Imag2, Iphase2);

	// swap phases and convert back to FFT images
	// (Ifft1, Ifft2 already have the right size and two float channels)
	HW_MagPhase2fft(Imag1, Iphase2, Ifft1);		// mag(I1), phase(I2)
	HW_MagPhase2fft(Imag2, Iphase1, Ifft2);		// mag(I2), phase(I1)

	// compute inverse FFT
	HW_fft2D(Ifft1, -1, IinvFFT1);
	HW_fft2D(Ifft2, -1, IinvFFT2);

	// extract magnitude from resulting images and save as uchar images
	magToUchar(IinvFFT1, II1);
	magToUchar(IinvFFT2, II2);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// magToUchar:
//
// Save the magnitudes of the complex values in Ifft as an 8-bit image Iout.
//
static void
magToUchar(ImagePtr Ifft, ImagePtr Iout)
{
	int w = Ifft->width ();
	int h = Ifft->height();

	// extract magnitude from complex image
	ImagePtr Imag = NEWIMAGE, Iphase = NEWIMAGE;
	HW_fft2MagPhase(Ifft, Imag, Iphase);

	// allocate uchar image and cast float channel to uchar
	Iout->allocImage(w, h, BW_TYPE);
	Iout->setImageType(BW_IMAGE);
	IP_castChannel(Imag, 0, Iout, 0, UCHAR_TYPE);
}
