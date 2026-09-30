#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{
  IP_copyImageHeader(I1, I2);
  int w = I1->width();
  int h = I1->height();
  int total = w * h;

// ORD
  int i, lut[MXGRAY];
for(i = 0; i < MXGRAY; i++) {
    double val = 255.0 * pow((double)i / 255.0, 1.0 / gamma);
    lut[i] = CLIP(val, 0, MaxGray);
}
// END

  ChannelPtr<uchar> p1, p2;
  int type;

  for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
    IP_getChannel(I2, ch, p2, type);
    for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; } // Apply LUT to pixels
  }

}
