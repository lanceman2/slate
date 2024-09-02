#include <stdbool.h>
#include <signal.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}


static int draw_count = 0;

static
int draw(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

    // Line stride (increment, pitch or step size) is the number of bytes
    // that one needs to add to the address in the first pixel of a row in
    // order to go to the address of the first pixel of the next row.
    draw_count++;
    fprintf(stderr, "               draw_count=%d    \r", draw_count);

    // Each pixel it 4 bytes or the sizeof(uint32_t) = 4 bytes.
    uint32_t *pix = pixels;
    // linePad is the distance in pixels to the end of a x row
    // from the last pixel drawn.  linePad is likely 0.
    // Note: stride >= w * 4 (width*4).
    const uint32_t linePad = stride/4 - w;
    for(uint32_t y=0; y<h; y++) {
        for(uint32_t x=0; x<w; ++x) {

            // ARGB color pix is for example
            // like 0x0AFF0022 is alpha=0A red=FF green=00 blue=22

            if(x > 300)
                *pix = 0xFFFF0000;
            else if(x > 120) 
                *pix = 0x0F0000FF;
            else
                *pix = 0xFF00FF00;

            // Go to next pixel.
            pix++;
        }
        // Go to next line.
        pix += linePad;
    }

    //if(draw_count >= 0)
    //    return 1; // stop calling

    return 0; //continue to calling at every frame, like at 60 Hz.
}



int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);


    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *w = slWindow_createToplevel(
            d, 500, 400, 10, 10, 0);
    if(!w) return 1; // fail

    struct SlWindow *p = slWindow_createPopup(
            w, 400, 300, -4400, -10000, draw);
    if(!p) return 1; // fail


    while(slDisplay_dispatch(d));

    return 0;
}
