// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_utils:
//
// Utility functions for converting between FFT coefficients and their
// magnitude and phase representations, and our own 1-D and 2-D FFT.
//

#include <cmath>
#include <complex>
#include <vector>
#include "IP.h"
using namespace IP;

typedef std::complex<double> complexD;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_fft2MagPhase:
//
// Convert a 2-channel FFT image into two 1-channel magnitude and phase images.
//
void
HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase)
{
	// init vars
	int w = Ifft->width ();
	int h = Ifft->height();
	int total = w*h;

	// allocate single-channel images for magnitude and phase
	Imag  ->allocImage(w, h, FLOATCH_TYPE);
	Iphase->allocImage(w, h, FLOATCH_TYPE);

	// get pointers for magnitude and phase channels
	ChannelPtr<float> mag   = Imag  [0];
	ChannelPtr<float> phase = Iphase[0];

	// get pointers for real and imaginary FFT channels
	ChannelPtr<float> real = Ifft[0];
	ChannelPtr<float> imag = Ifft[1];

	// compute mag = sqrt(real^2 + imag^2) and phase = atan2(imag/real)
	for(int i=0; i<total; ++i) {
		mag  [i] = sqrt (real[i]*real[i] + imag[i]*imag[i]);
		phase[i] = atan2(imag[i], real[i]);
	}
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_MagPhase2fft:
//
// Convert 1-channel magnitute and phase images into a 2-channel FFT image.
//
void
HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft)
{
	// init vars
	int w = Ifft->width();
	int h = Ifft->height();
	int total = w*h;

	// get pointers for magnitude and phase channels
	ChannelPtr<float> mag   = Imag  [0];
	ChannelPtr<float> phase = Iphase[0];

	// get pointers for real and imaginary FFT channels
	ChannelPtr<float> real = Ifft[0];
	ChannelPtr<float> imag = Ifft[1];

	// compute real and imaginary values from magnitude and phase
	for(int i=0; i<total; ++i) {
		real[i] = mag[i] * cos(phase[i]);
		imag[i] = mag[i] * sin(phase[i]);
	}
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// fftRecursive:
//
// Recursive Danielson-Lanczos FFT of the N complex values in a (N a power of 2).
// The DFT of length N is split into the DFTs of the even and odd samples:
//	F[n]       = Fe[n] + W^n Fo[n]
//	F[n + N/2] = Fe[n] - W^n Fo[n],   W = e^(sign*i*2*PI/N)
// sign = -1 for the forward transform, +1 for the inverse. No scaling here.
//
static void
fftRecursive(std::vector<complexD> &a, int sign)
{
	int N = a.size();
	if(N == 1) return;

	// split into even and odd samples, and transform each half
	std::vector<complexD> even(N/2), odd(N/2);
	for(int k=0; k<N/2; k++) {
		even[k] = a[2*k];
		odd [k] = a[2*k + 1];
	}
	fftRecursive(even, sign);
	fftRecursive(odd , sign);

	// combine the two halves
	for(int n=0; n<N/2; n++) {
		complexD Wn = std::polar(1.0, sign * 2*M_PI*n / N);
		a[n]       = even[n] + Wn * odd[n];
		a[n + N/2] = even[n] - Wn * odd[n];
	}
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// dft:
//
// Straightforward O(N^2) DFT, used when N is not a power of 2.
//
static void
dft(std::vector<complexD> &a, int sign)
{
	int N = a.size();
	std::vector<complexD> F(N);
	for(int n=0; n<N; n++) {
		complexD sum = 0;
		for(int k=0; k<N; k++) sum += a[k] * std::polar(1.0, sign * 2*M_PI*n*k / N);
		F[n] = sum;
	}
	a = F;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_fft1D:
//
// 1-D FFT of the N complex values in a.
// dir = 1: forward transform, F[n] = 1/N sum f[k] e^(-i2PI nk/N)
// dir = -1: inverse transform, f[k] = sum F[n] e^(+i2PI nk/N)
//
void
HW_fft1D(std::vector<complexD> &a, int dir)
{
	int N = a.size();
	int sign = (dir == 1) ? -1 : 1;

	if((N & (N-1)) == 0)	fftRecursive(a, sign);	// N is a power of 2
	else			dft(a, sign);

	if(dir == 1) for(int i=0; i<N; i++) a[i] /= N;	// forward: scale by 1/N
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_fft2D:
//
// Separable 2-D FFT: apply HW_fft1D to all rows, then to all columns of
// the result. Input I1 is either a 1-channel uchar image (an imaginary
// channel of 0's is appended) or a 2-channel FFT image (real, imaginary).
// Output Ifft is a 2-channel float FFT image.
// dir = 1: forward transform; dir = -1: inverse transform.
//
void
HW_fft2D(ImagePtr I1, int dir, ImagePtr Ifft)
{
	int w = I1->width ();
	int h = I1->height();

	// copy input into a complex array, appending zero imaginary parts if needed
	std::vector<complexD> c(w*h);
	int type;
	ChannelPtr<float> re, im;
	if(I1->imageType() == FFT_IMAGE) {
		IP_getChannel(I1, 0, re, type);
		IP_getChannel(I1, 1, im, type);
		for(int i=0; i<w*h; i++) c[i] = complexD(re[i], im[i]);
	} else {
		ChannelPtr<uchar> p;
		IP_getChannel(I1, 0, p, type);
		for(int i=0; i<w*h; i++) c[i] = complexD(p[i], 0);
	}

	// pass 1: transform each row
	std::vector<complexD> line(w);
	for(int y=0; y<h; y++) {
		for(int x=0; x<w; x++) line[x] = c[y*w + x];
		HW_fft1D(line, dir);
		for(int x=0; x<w; x++) c[y*w + x] = line[x];
	}

	// pass 2: transform each column of the intermediate result
	line.resize(h);
	for(int x=0; x<w; x++) {
		for(int y=0; y<h; y++) line[y] = c[y*w + x];
		HW_fft1D(line, dir);
		for(int y=0; y<h; y++) c[y*w + x] = line[y];
	}

	// save result as a 2-channel (real, imaginary) FFT image
	Ifft->allocImage(w, h, FFT_TYPE);
	Ifft->setImageType(FFT_IMAGE);
	ChannelPtr<float> outRe = Ifft[0];
	ChannelPtr<float> outIm = Ifft[1];
	for(int i=0; i<w*h; i++) {
		outRe[i] = c[i].real();
		outIm[i] = c[i].imag();
	}
}
