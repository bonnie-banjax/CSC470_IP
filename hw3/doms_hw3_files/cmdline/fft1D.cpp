// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// fft1D:
//
// Usage: fft1D in dir out
//
// Same as dft1D, but computes the transform with the recursive
// Danielson-Lanczos FFT. N must be a power of 2.
// dir = 0: forward FFT	F[n] = 1/N sum_k f[k] e^(-i2PI nk/N)
// dir = 1: inverse FFT	f[n] =     sum_k F[k] e^(+i2PI nk/N)
//
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <vector>

typedef std::complex<double> complexD;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// fft:
//
// Danielson-Lanczos lemma: a DFT of length N is the sum of two DFTs of
// length N/2, one of the even samples (Fe) and one of the odd samples (Fo):
//	F[n]       = Fe[n] + W^n Fo[n]
//	F[n + N/2] = Fe[n] - W^n Fo[n],   W = e^(sign*i2PI/N),  0 <= n < N/2
// Recurse until the length is 1 (the DFT of one sample is the sample itself).
//
static void
fft(std::vector<complexD> &a, int sign)
{
	int N = a.size();
	if(N == 1) return;

	// split into even and odd samples, and transform each half
	std::vector<complexD> even(N/2), odd(N/2);
	for(int k=0; k<N/2; k++) {
		even[k] = a[2*k];
		odd [k] = a[2*k + 1];
	}
	fft(even, sign);
	fft(odd , sign);

	// combine the two halves
	for(int n=0; n<N/2; n++) {
		complexD Wn = std::polar(1.0, sign * 2*M_PI*n / N);
		a[n]       = even[n] + Wn * odd[n];
		a[n + N/2] = even[n] - Wn * odd[n];
	}
}

int
main(int argc, char **argv)
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s in dir out   (dir: 0=forward, 1=inverse)\n", argv[0]);
		return 1;
	}
	int dir = atoi(argv[2]);

	// read input list: header "2 N", then N lines of "real imag"
	FILE *fp = fopen(argv[1], "r");
	if(!fp) { fprintf(stderr, "Can't open %s\n", argv[1]); return 1; }
	int w, N;
	if(fscanf(fp, "%d %d", &w, &N) != 2 || N <= 0) { fprintf(stderr, "Bad header in %s\n", argv[1]); return 1; }
	if(N & (N-1)) { fprintf(stderr, "N = %d is not a power of 2\n", N); return 1; }
	std::vector<complexD> a(N);
	for(int k=0; k<N; k++) {
		double r, i = 0;
		if(w == 1) fscanf(fp, "%lf", &r);			// real-only input
		else	   fscanf(fp, "%lf %lf", &r, &i);
		a[k] = complexD(r, i);
	}
	fclose(fp);

	// forward: e^(-i...) and scale by 1/N; inverse: e^(+i...), no scaling
	fft(a, (dir == 0) ? -1 : 1);
	if(dir == 0) for(int n=0; n<N; n++) a[n] /= N;

	// write output list in the same format
	fp = fopen(argv[3], "w");
	if(!fp) { fprintf(stderr, "Can't create %s\n", argv[3]); return 1; }
	fprintf(fp, "2 %d\n", N);
	for(int n=0; n<N; n++) {
		double r = (fabs(a[n].real()) < 1e-10) ? 0 : a[n].real();	// print tiny round-off as 0
		double i = (fabs(a[n].imag()) < 1e-10) ? 0 : a[n].imag();
		fprintf(fp, "%.10f %.10f\n", r, i);
	}
	fclose(fp);
	return 0;
}
