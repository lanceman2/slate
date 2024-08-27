#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}


static int color = 100;

static int draw_count = 0;

static
int draw(struct SlWindow *win, void *pixels,
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

            if(x > 400)
                *pix = 0x0AFF0000;
            else if(x > 220) 
                *pix = 0x0A0000FF;
            else
                *pix = 0x0A00FF00;

            // Go to next pixel.
            pix++;
        }
        // Go to next line.
        pix += linePad;
    }
    

    //memset(pixels, color, w*h*4);

    color++;

    if(draw_count >= 200)
        return 1; // stop calling

    return 0; //continue to calling at every frame, like at 60 Hz.
}

int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createTop(d, 600, 600, 100, 10, 0/*draw()*/);
    if(!win) return 1; // fail

    fprintf(stderr, "\n\nPress Key <Alt-F4> to exit\n\n");

    slWindow_setDraw(win, draw);

    while(slDisplay_dispatch(d));


    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seen to crash.
    DSPEW("DONE");

    return 0;
}
