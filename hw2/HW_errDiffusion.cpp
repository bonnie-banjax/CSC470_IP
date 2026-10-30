#include <algorithm>
#include <cmath>
#include <vector>
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//
void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
	// Keep the source intact if input and output refer to the same image.
	if(I1 == I2) {
		ImagePtr source;
		IP_copyImage(I1, source);
		I1 = source;
	}
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	if(w <= 0 || h <= 0) return;

	struct Tap {
		int dx, dy, weight;
	};
	const Tap floydSteinberg[] = {
		{1, 0, 7}, {-1, 1, 3}, {0, 1, 5}, {1, 1, 1}
	};
	const Tap jarvisJudiceNinke[] = {
		{1, 0, 7}, {2, 0, 5},
		{-2, 1, 3}, {-1, 1, 5}, {0, 1, 7}, {1, 1, 5}, {2, 1, 3},
		{-2, 2, 1}, {-1, 2, 3}, {0, 2, 5}, {1, 2, 3}, {2, 2, 1}
	};
	bool jarvis = method == 1;
	const Tap *taps = jarvis ? jarvisJudiceNinke : floydSteinberg;
	int tapCount = jarvis ? 12 : 4;
	int denominator = jarvis ? 48 : 16;
	int padding = jarvis ? 2 : 1;
	int rows = padding + 1;
	int stride = w + 2 * padding;

	// Match HW_gammaCorrect's reciprocal-gamma convention.
	// Invalid gamma values fall back to the identity correction.
	if(!std::isfinite(gamma) || gamma <= 0) gamma = 1;
	short lut[MXGRAY];
	for(int v=0; v<MXGRAY; v++) {
		double corrected = MaxGray * std::pow((double) v / MaxGray, 1.0 / gamma);
		lut[v] = ROUND(corrected);
	}

	std::vector<short> buffer((size_t) rows * stride);
	ChannelPtr<uchar> p1, p2;
	int type;
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		std::fill(buffer.begin(), buffer.end(), 0);
		for(int y=0; y<rows && y<h; y++) {
			short *row = &buffer[(size_t) y * stride];
			for(int x=0; x<w; x++) row[x + padding] = lut[p1[y*w + x]];
		}

		for(int y=0; y<h; y++) {
			short *current = &buffer[(size_t) (y % rows) * stride];
			int direction = (serpentine && y % 2) ? -1 : 1;
			int start = direction == 1 ? 0 : w - 1;
			int end = direction == 1 ? w : -1;
			for(int x=start; x!=end; x+=direction) {
				int value = current[x + padding];
				int output = value > MaxGray / 2.0 ? MaxGray : 0;
				p2[y*w + x] = output;
				int error = value - output;
				for(int t=0; t<tapCount; t++) {
					int yy = y + taps[t].dy;
					if(yy >= h) continue;
					int xx = x + direction * taps[t].dx;
					short *destination = &buffer[(size_t) (yy % rows) * stride];
					// Side padding absorbs outgoing error and is never processed.
					destination[xx + padding] += ROUND((double) error * taps[t].weight / denominator);
				}
			}

			// Reuse the completed row for the next incoming input row.
			std::fill(current, current + stride, 0);
			int incoming = y + rows;
			if(incoming < h) {
				for(int x=0; x<w; x++) current[x + padding] = lut[p1[incoming*w + x]];
			}
		}
	}
}
