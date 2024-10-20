#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <inttypes.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}


static uint32_t draw_count = 0;

static
int draw(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

    // Line stride (increment, pitch or step size) is the number of bytes
    // that one needs to add to the address in the first pixel of a row in
    // order to go to the address of the first pixel of the next row.
    draw_count++;
    fprintf(stderr, "               draw_count=%" PRIu32 "   \r",
            draw_count);

    uint32_t midColor = 0xC10000FA;

#define FRAMES  100

    if((draw_count % (FRAMES*2)) < FRAMES)
        midColor = 0x00010101;

    // Each pixel it 4 bytes or the sizeof(uint32_t) = 4 bytes.
    uint32_t *pix = pixels;
    // linePad is the distance in pixels to the end of a x row
    // from the last pixel drawn.  linePad is likely 0.
    // Note: stride >= w * 4    (is width*4).
    const uint32_t linePad = stride/4 - w;
    for(uint32_t y=0; y<h; y++) {
        for(uint32_t x=0; x<w; ++x) {

            // ARGB color pix is for example:
            // 0x0AFF0022 is alpha=0A red=FF green=00 blue=22 (in hex)

            if(x > 400)
                *pix = 0x0AFF0000;
            else if(x > 220 && y < 500)
                *pix = midColor;
            else
                *pix = 0xFF00FF00;

            // Go to next pixel.
            pix++;
        }
        // Go to next line.
        pix += linePad;
    }
 
    // Continue to calling at every frame, like at 60 Hz.
    return SlDrawReturn_frame;
}


int main(void) {

    // We need to know SLATE_PIXEL_SIZE does not change, so that we know
    // how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createToplevel(d,
            600, 600, 100, 10, draw/*draw()*/, 0, true/*showing*/);
    if(!win) return 1; // fail

    fprintf(stderr, "\nPress Key <Alt-F4> on the window to exit\n\n");

    while(slDisplay_dispatch(d));

    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seem to crash.
    DSPEW("DONE");

    return 0;
}
