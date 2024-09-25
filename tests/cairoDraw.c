#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include <cairo.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}


static
int draw(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            (unsigned char *) pixels, CAIRO_FORMAT_ARGB32,
            w, h, stride);


    cairo_surface_destroy(surface);


    return 1; // stop calling

    //return 0; //continue to calling at every frame, like at 60 Hz.
}

int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createToplevel(d,
            600, 600, 100, 10, draw/*draw()*/);
    if(!win) return 1; // fail

    fprintf(stderr, "\n\nPress Key <Alt-F4> to exit\n\n");

    //slWindow_setDraw(win, draw);

    while(slDisplay_dispatch(d));


    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seem to crash.
    DSPEW("DONE");

    return 0;
}
