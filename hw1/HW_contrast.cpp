#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_contrast:
//
// Apply contrast enhancement to I1. Output is in I2.
// Stretch intensity difference from reference value (128) by multiplying
// difference by "contrast" and adding it back to 128. Shift result by
// adding "brightness" value.
//
void
HW_contrast(ImagePtr I1, double brightness, double contrast, ImagePtr I2)
{

  IP_copyImageHeader(I1, I2);
  int w = I1->width();
  int h = I1->height();
  int total = w * h;

// ORD
  int i, lut[MXGRAY];
for(i = 0; i < MXGRAY; i++) {
    double val = (i - 128.0) * contrast + 128.0 + brightness;
    lut[i] = CLIP(val, 0, MaxGray); // CLIP macro handles 0-255 bounds
}
// END

  ChannelPtr<uchar> p1, p2;
  int type;

  for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
    IP_getChannel(I2, ch, p2, type);
    for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; } // Apply LUT to pixels
  }

}
