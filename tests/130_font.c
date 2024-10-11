#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}



static
int draw(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

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

    DSPEW("Finished drawing");

    return SlDrawReturn_configure;
}

int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createToplevel(d,
            600, 600, 100, 10, draw/*draw()*/, 0, true/*showing*/);
    if(!win) return 1; // fail

    if(slWindow_DrawText(win,
            "Hgello World!", "Mono",
            0/*x*/, 120/*y*/, 1560/*width*/, 120/*height*/,
            /*angle in radians*/
            0,
            //- 30.0 * M_PI/180.0,
            0xF0FF0000/*bgColor*/, 0xFF0009FF/*fgColor*/))
        return 1; // fail

#ifdef LOOP
    fprintf(stderr, "\n\nPress Key <Alt-F4> to exit\n\n");
    while(slDisplay_dispatch(d));
#endif

    DSPEW("DONE");

    return 0;
}
