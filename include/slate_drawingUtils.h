#include <stdint.h>

// "pix" is a (uint32_t *) pointer to the top left corner of the
// drawing surface.
//
// TODO: Does libpixman.so do this kind of thing better?  ...automatically
// using the hardware specific array operations (like SIMD).  The nice
// thing about this code is that it is easy to follow (at least for me).
//
static inline
void sl_drawFilledRectangle(uint32_t *pix/*surface starting pixel*/,
        uint32_t x, uint32_t y,
        uint32_t width, uint32_t height, uint32_t stride,
        uint32_t color) {

#ifdef SLATE_LIB_CODE
    DASSERT(pix);
    DASSERT(width);
    DASSERT(height);
    DASSERT(stride);
    DASSERT(stride % 4 == 0);
#endif

    stride /= 4;

    pix += x + y * stride;

    // "end" is a pointer to a pixel after the last pixel (if it exists).
    // "end" is just a limiting pointer that we never dereference.
    //
    for(uint32_t *end = pix + height * stride;
            pix < end; pix += stride)
        for(uint32_t *rowEnd = pix + width; pix < rowEnd; ++pix)
            *pix = color;
}
