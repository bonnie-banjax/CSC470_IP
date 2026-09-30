#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_clip:
//
// Clip intensities of image I1 to [t1,t2] range. Output is in I2.
// If    input<t1: output = t1;
// If t1<input<t2: output = input;
// If      val>t2: output = t2;
//
void
HW_clip(ImagePtr I1, int t1, int t2, ImagePtr I2)
{

  // 1. Boilerplate: Copy header & get dimensions
  IP_copyImageHeader(I1, I2);
  int w = I1->width();
  int h = I1->height();
  int total = w * h;

  // 2. Core Logic: Build the Lookup Table
  int i, lut[MXGRAY];
  for(i = 0; i < MXGRAY; i++) {
    if   (i < t1) lut[i] = t1;
    else if (i > t2) lut[i] = t2;
    else       lut[i] = i;
  }

  // 3. Boilerplate: Channel and Pixel Iteration
  ChannelPtr<uchar> p1, p2;
  int type;
  for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
    IP_getChannel(I2, ch, p2, type);
    for(i = 0; i < total; i++) {
      *p2++ = lut[*p1++]; // Apply LUT to pixels
    }
  }

}
