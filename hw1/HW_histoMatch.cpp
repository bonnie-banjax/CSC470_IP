#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);
void scaleTargetHisto(ImagePtr, int, int*);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
// The target histogram is given in targetHisto (MXGRAY int entries).
// approxAlg=1: simple (non-exact) matching from the textbook (Algorithm 3.10).
// approxAlg=0: exact matching based on the code in the notes. Each input gray
//		level v is assigned an interval of output levels [left[v], right[v]].
//		Pixels with value v fill these output levels in order, each up to
//		its quota, so the output histogram equals the target histogram
//		(except possibly the last entry, which absorbs rounding error).
//
void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if(approxAlg) {
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// get target histogram, scaled so that its entries sum to total
	int histo2[MXGRAY];
	scaleTargetHisto(targetHisto, total, histo2);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;
	int i, v, histo1[MXGRAY];

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

		// compute histogram of input channel
		for(i=0; i<MXGRAY; i++) histo1[i] = 0;		// clear histogram
		for(i=0; i<total;  i++) histo1[p1[i]]++;	// eval  histogram

		// evaluate remapping of all input gray levels: input level v maps
		// to the interval [left[v], right[v]] of output levels.
		// quota[v] is how many pixels of value v may still go to output
		// level left[v]; Hleft[v] is how many pixels of value v remain.
		int  left[MXGRAY], right[MXGRAY], quota[MXGRAY], Hleft[MXGRAY];
		int  r    = 0;			// current output level
		long room = histo2[0];		// unfilled space in output level r
		for(v=0; v<MXGRAY; v++) {
			// skip output levels that are already full (or empty in target)
			while(room == 0 && r < MaxGray) room = histo2[++r];

			left [v] = r;					// left end of interval
			quota[v] = (int) MIN(room, (long) histo1[v]);	// share of level r for v
			Hleft[v] = histo1[v];				// pixels of value v left

			// widen interval until all pixels of value v are placed
			long Hsum = histo1[v];
			while(Hsum > room && r < MaxGray) {
				Hsum -= room;			// fill level r completely
				room  = histo2[++r];		// move on to next output level
			}
			room -= Hsum;				// remaining space in level r
			if(room < 0) room = 0;			// last level absorbs overflow
			right[v] = r;				// right end of interval
		}

		// visit all input pixels and remap the intensities
		for(i=0; i<total; i++) {
			v = p1[i];

			// current output level is full for this input level: advance
			while(quota[v] == 0 && left[v] < right[v]) {
				left [v]++;
				quota[v] = MIN(histo2[left[v]], Hleft[v]);
			}
			p2[i] = left[v];
			if(quota[v] > 0) quota[v]--;
			Hleft[v]--;
		}
	}
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// histoMatchApprox:
//
// Apply approximate histogram matching to I1. Output is in I2.
// Based on Algorithm 3.10 (HistogramMatch) in the textbook: map each input
// level l to the largest output level l' whose target CDF does not exceed
// the CDF of l in the input image. All pixels of value l map to one level,
// so the output histogram only approximates the target histogram.
//
void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// get target histogram, scaled so that its entries sum to total
	int histo2[MXGRAY];
	scaleTargetHisto(targetHisto, total, histo2);

	// evaluate CDF of target histogram (running sum, normalized to [0,1])
	int i;
	double cRef[MXGRAY];
	long sum = 0;
	for(i=0; i<MXGRAY; i++) {
		sum    += histo2[i];
		cRef[i] = (double) sum / total;
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;
	int histo1[MXGRAY], lut[MXGRAY];
	double c[MXGRAY];

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

		// compute histogram of input channel
		for(i=0; i<MXGRAY; i++) histo1[i] = 0;		// clear histogram
		for(i=0; i<total;  i++) histo1[p1[i]]++;	// eval  histogram

		// evaluate CDF of input channel (running sum, normalized to [0,1])
		sum = 0;
		for(i=0; i<MXGRAY; i++) {
			sum += histo1[i];
			c[i] = (double) sum / total;
		}

		// init lookup table: for each level l, find l' such that cRef[l'] ~= c[l].
		// Start l' at the maximum gray level and decrement until cRef[l'] <= c[l].
		for(int l=0; l<MXGRAY; l++) {
			int lp = MaxGray;
			do {
				lut[l] = lp;
				lp--;
			} while(lp >= 0 && cRef[lp] > c[l]);
		}

		for(i=0; i<total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// scaleTargetHisto:
//
// Copy the MXGRAY entries of targetHisto into histo2[], scaled so that
// they sum to total (the number of pixels in the input image).
// Rounding errors are corrected so that the sum is exactly total:
// an overshoot is clamped (remaining entries cleared), and any shortfall
// is added to the very last entry.
//
void
scaleTargetHisto(ImagePtr targetHisto, int total, int *histo2)
{
	// get pointer to target histogram data
	ChannelPtr<int> lutp;
	int type;
	IP_getChannel(targetHisto, 0, lutp, type);

	// compute sum of target histogram for normalization
	int i;
	double total2 = 0;
	for(i=0; i<MXGRAY; i++) total2 += lutp[i];

	// a target histogram with no entries is treated as a flat histogram
	if(total2 <= 0) {
		for(i=0; i<MXGRAY; i++) histo2[i] = total / MXGRAY;
		histo2[MaxGray] += total - (total / MXGRAY) * MXGRAY;
		return;
	}

	// scale target histogram to conform with dimensions of I1
	double scale = total / total2;
	long sum = 0;
	for(i=0; i<MXGRAY; i++) {
		histo2[i] = ROUND(lutp[i] * scale);

		// update histo2[] if cumulative histogram overshoots due to rounding
		sum += histo2[i];
		if(sum > total) {				// check for overshoot
			histo2[i] -= (int) (sum - total);	// clamp last non-zero histo2[]
			for(i++; i<MXGRAY; i++) histo2[i] = 0;	// clear remainder of histo2[]
			sum = total;
		}
	}

	// add any shortfall due to rounding to the very last entry
	histo2[MaxGray] += (int) (total - sum);
}
