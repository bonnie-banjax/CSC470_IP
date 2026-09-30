#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
  IP_copyImageHeader(I1, I2);
  int w = I1->width();
  int h = I1->height();
  int total = w * h;

// ORD
  int i, lut[MXGRAY];
for(i = 0; i < MXGRAY; i++) {
    if      (i <= t1) lut[i] = 0;
    else if (i >= t2) lut[i] = MaxGray;
    else              lut[i] = (int)(((i - t1) * 255.0) / (t2 - t1) + 0.5);
}
// END

  ChannelPtr<uchar> p1, p2;
  int type;

  for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
    IP_getChannel(I2, ch, p2, type);
    for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; } // Apply LUT to pixels
  }

}
