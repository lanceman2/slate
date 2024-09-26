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

#if 0
    const int NUM = 20;

    cairo_t *c[NUM];
    for(int i = 0; i<NUM; ++i) {
        c[i] = cairo_create(surface);
        ERROR("cr=%p", c[i]);
        ASSERT(c[i]);
    }

    for(int i = 0; i<NUM; ++i)
        cairo_destroy(c[i]);
#endif

    cairo_t *cr = cairo_create(surface);
ERROR("cr=%p", cr);
    cairo_surface_destroy(surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    cairo_set_source_rgba(cr, 0, 0.9, 0, 0.4);

    cairo_paint(cr);

    cairo_destroy(cr);

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

#ifdef LOOP
    while(slDisplay_dispatch(d));
#endif

    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seem to crash.
    DSPEW("DONE");


    // This is fucking AWESOME.  Cairo has a robust memory cleanup method:
    // in the function cairo_debug_reset_static_data().  I was working on
    // a patch to fix the memory leaks in libcairo.so and came upon this
    // after a half day of working.
    //
    // After adding this function call this program passes the Valgrind
    // memory allocate/free test.  It is only safe to call this function
    // when there are no active cairo objects remaining.  One would think
    // it would figure out what to do, otherwise what's the point of
    // having a computer.  I just have to cleanup my cairo code myself,
    // before calling this; not like I did with the libslate.so library
    // destructor (which cleans all things slate).
    //
    cairo_debug_reset_static_data();

    return 0;
}
