#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//



void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
  IP_copyImageHeader(I1, I2);
  int w = I1->width();
  int h = I1->height();
  int total = w * h;
  int step = MaxGray / (levels - 1);

  ChannelPtr<uchar> p1, p2;
  int i, type;

  // Lambda for the non-dithered LUT path
  auto quantizeLUT = [&]() {

    int lut[MXGRAY];
    for(i = 0; i < MXGRAY; i++) {
      int levelIdx = (i * levels) / MXGRAY;
      lut[i] = CLIP(levelIdx * step, 0, MaxGray);
    }

    for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
      IP_getChannel(I2, ch, p2, type);
      for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; }
    }

  };

  struct {
    // References mirroring the [&] capture list
    int& levels;
    int& step;
    int& total;
    int& i;
    ImagePtr I1; // apparently functionally same as references, but...
    ImagePtr I2; // same as above, undecided on whether to change
    ChannelPtr<uchar>& p1;
    ChannelPtr<uchar>& p2;
    int& type;

    // The lambda body becomes operator()
    void operator()() const {
      for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        for(i = 0; i < total; i++) {
          int val = *p1++;
          int noise = ((rand() & 0x7fff) / 16384.0 - 0.5) * step;
          val = CLIP(val + noise, 0, MaxGray);
          int levelIdx = (val * levels) / MXGRAY;
          *p2++ = CLIP(levelIdx * step, 0, MaxGray);
        }
      }
    }
  } quantizeDither{levels, step, total, i, I1, I2, p1, p2, type};

  // Route based on flag
  if (dither) quantizeDither();
  else           quantizeLUT();

  // NOTE: IN PROTEST OF THE FUNCTION SIGNATURE!
  // switch (dither) {
  //   case  true: quantizeDither() ; break;
  //   case false: quantizeLUT()    ; break;
  // }

}


// void
// HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
// {
//   IP_copyImageHeader(I1, I2);
//   int w = I1->width();
//   int h = I1->height();
//   int total = w * h;
//   int step = MaxGray / (levels - 1);
//
//   ChannelPtr<uchar> p1, p2;
//   int i, type;
//
//   // Lambda for the non-dithered LUT path
//   auto quantizeLUT = [&]() {
//
//     int lut[MXGRAY];
//     for(i = 0; i < MXGRAY; i++) {
//       int levelIdx = (i * levels) / MXGRAY;
//       lut[i] = CLIP(levelIdx * step, 0, MaxGray);
//     }
//
//     for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
//       IP_getChannel(I2, ch, p2, type);
//       for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; }
//     }
//
//   };
//
//   // Lambda for the dithered per-pixel path
//   auto quantizeDither = [&]() {
//     for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
//       IP_getChannel(I2, ch, p2, type);
//       for(i = 0; i < total; i++) {
//         int val = *p1++;
//         int noise = ((rand() & 0x7fff) / 16384.0 - 0.5) * step;
//         val = CLIP(val + noise, 0, MaxGray);
//         int levelIdx = (val * levels) / MXGRAY;
//         *p2++ = CLIP(levelIdx * step, 0, MaxGray);
//       }
//     }
//   };
//
//   // Route based on flag
//   if (dither) quantizeDither();
//   else           quantizeLUT();
//
// }

// void
// HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
// {
//
// // need to implement the random dithering, which prevents using a LUT
//
//   IP_copyImageHeader(I1, I2);
//   int w = I1->width();
//   int h = I1->height();
//   int total = w * h;
//
// // ORD
//   int i, lut[MXGRAY];
// int step = MaxGray / (levels - 1);
// for(i = 0; i < MXGRAY; i++) {
//     int levelIdx = (i * levels) / MXGRAY;
//     lut[i] = CLIP(levelIdx * step, 0, MaxGray);
// }
// // END
//
//   ChannelPtr<uchar> p1, p2;
//   int type;
//
//   for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
//     IP_getChannel(I2, ch, p2, type);
//     for(i = 0; i < total; i++) { *p2++ = lut[*p1++]; } // Apply LUT to pixels
//   }
//
// }


// void
// HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
// {
//   // 1. Boilerplate: Copy header & get dimensions
//   IP_copyImageHeader(I1, I2);
//   int w = I1->width();
//   int h = I1->height();
//   int total = w * h;
//
//   ChannelPtr<uchar> p1, p2;
//   int type;
//   int step = MaxGray / (levels - 1);
//
//   // 2. Top-level branch based on dither flag
//   if (!dither) {
//     // --- NON-DITHERED PATH: Use Lookup Table ---
//     int i, lut[MXGRAY];
//     for(i = 0; i < MXGRAY; i++) {
//       int levelIdx = (i * levels) / MXGRAY;
//       lut[i] = CLIP(levelIdx * step, 0, MaxGray);
//     }
//
//     for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
//       IP_getChannel(I2, ch, p2, type);
//       for(i = 0; i < total; i++) {
//         *p2++ = lut[*p1++];
//       }
//     }
//   } else {
//     // --- DITHERED PATH: Dynamic per-pixel noise calculation ---
//     for(int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
//       IP_getChannel(I2, ch, p2, type);
//       for(int i = 0; i < total; i++) {
//         int val = *p1++;
//
//         // Add random noise offset
//         int noise = ((rand() & 0x7fff) / 16384.0 - 0.5) * step;
//         val = CLIP(val + noise, 0, MaxGray);
//
//         // Quantize to nearest level matching the indexing logic
//         int levelIdx = (val * levels) / MXGRAY;
//         *p2++ = CLIP(levelIdx * step, 0, MaxGray);
//       }
//     }
//   }
// }
