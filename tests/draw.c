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

draw_count++;
fprintf(stderr, "               draw_count=%d    \r", draw_count);

    memset(pixels, color, w*h*4);

    color++;

    //return 1; // stop calling

    return 0; //continue to calling at every frame, like at 60 Hz.
}

int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createTop(d, 100, 100, 100, 10, 0/*draw()*/);
    if(!win) return 1; // fail

    fprintf(stderr, "\n\nKey Press <Alt-F4> to exit\n\n");

    slWindow_setDraw(win, draw);

    while(slDisplay_dispatch(d));


    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seen to crash.
    DSPEW("DONE");

    return 0;
}
