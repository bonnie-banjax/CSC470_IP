// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// dft1D:
//
// Usage: dft1D in dir out
//
// Compute the 1-D discrete Fourier transform of the list of complex numbers
// in file in, and save the result in file out.
// dir = 0: forward DFT	F[n] = 1/N sum_k f[k] e^(-i2PI nk/N)
// dir = 1: inverse DFT	f[n] =     sum_k F[k] e^(+i2PI nk/N)
//
// File format (ASCII): first line holds the width and height of the data,
// i.e., 2 and N. Each of the next N lines holds one complex number as two
// columns: real and imaginary. N need not be a power of 2.
//
#include <cstdio>
#include <cstdlib>
#include <cmath>

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
	double *fr = new double[N], *fi = new double[N];
	for(int k=0; k<N; k++) {
		fi[k] = 0;
		if(w == 1) fscanf(fp, "%lf", &fr[k]);			// real-only input
		else	   fscanf(fp, "%lf %lf", &fr[k], &fi[k]);
	}
	fclose(fp);

	// straightforward DFT: N output terms, each a sum over all N inputs
	double *Fr = new double[N], *Fi = new double[N];
	double sign = (dir == 0) ? -1 : 1;			// e^(-i...) forward, e^(+i...) inverse
	for(int n=0; n<N; n++) {
		double real = 0, imag = 0;
		for(int k=0; k<N; k++) {
			double c = cos(sign * 2*M_PI*n*k / N);
			double s = sin(sign * 2*M_PI*n*k / N);
			// (fr + i fi)(c + i s) = (fr c - fi s) + i(fr s + fi c)
			real += fr[k]*c - fi[k]*s;
			imag += fr[k]*s + fi[k]*c;
		}
		if(dir == 0) { real /= N; imag /= N; }		// 1/N on the forward transform
		Fr[n] = real;
		Fi[n] = imag;
	}

	// write output list in the same format
	fp = fopen(argv[3], "w");
	if(!fp) { fprintf(stderr, "Can't create %s\n", argv[3]); return 1; }
	fprintf(fp, "2 %d\n", N);
	for(int n=0; n<N; n++) {
		double r = (fabs(Fr[n]) < 1e-10) ? 0 : Fr[n];	// print tiny round-off as 0
		double i = (fabs(Fi[n]) < 1e-10) ? 0 : Fi[n];
		fprintf(fp, "%.10f %.10f\n", r, i);
	}
	fclose(fp);

	delete [] fr; delete [] fi; delete [] Fr; delete [] Fi;
	return 0;
}
