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

    // CAUTION: Do not write your code like this.  This is just a test
    // program that has been diddled with quite a bit.

    ASSERT(stride % 4 == 0);

    stride /= 4; // in number of 4 byte ints

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            (unsigned char *) pixels, CAIRO_FORMAT_ARGB32,
            w, h, stride*4);

    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    // cr will keep a reference to surface.

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0, 0.9, 0.9, 0.8);
    cairo_paint(cr);
    cairo_destroy(cr);
 
    return 1; // stop calling

    //return 0; //continue to calling at every frame, like at 60 Hz.
}


int main(void) {

    // We need to know SLATE_PIXEL_SIZE does not change, so that we know
    // how to draw on pixels.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *win = slWindow_createToplevel(d,
            600, 600, 100, 10, draw/*draw()*/);
    if(!win) return 1; // fail

    slWidget_create(slWindow_getSurface(win), 200, 100,
            SlGravity_None/*SlGravity_None => non-container*/,
            SlGreed_None,
            0x20F0F000/* background color*/,
            0/* borderWidth*/,
            draw, true/*hide*/);

    fprintf(stderr, "\n\nPress Key <Alt-F4> to exit\n\n");

    //slWindow_setDraw(win, draw);

#ifdef LOOP
    while(slDisplay_dispatch(d));
#endif

    slDisplay_destroy(d);

    cairo_debug_reset_static_data();

    return 0;
}
