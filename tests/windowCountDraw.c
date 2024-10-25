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
int draw(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

    draw_count++;
    fprintf(stderr, "               draw_count=%d    \r", draw_count);

    memset(pixels, color, 4*w*h);

    color++;

    return SlDrawReturn_frame; //continue to calling at every frame, like at 60 Hz.
}

int main(void) {

    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    if(!slWindow_createToplevel(d, 100, 100, 100, 10, draw, 0, SL_SHOWING))
        return 1; // fail

    fprintf(stderr, "\n\nPress Key <Alt-F4> to exit\n\n");

    while(slDisplay_dispatch(d));


    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seem to crash.
    DSPEW("DONE");

    return 0;
}
