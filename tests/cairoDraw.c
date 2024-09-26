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

    ASSERT(stride % 4 == 0);

    stride /= 4; // in number of 4 bytes ints

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            (unsigned char *) pixels, CAIRO_FORMAT_ARGB32,
            w, h, stride*4);

    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    // cr will keep a reference to surface.

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0, 0.9, 0, 0.94);
    cairo_paint(cr);
    cairo_destroy(cr);
    // Now the surface is destroyed too.

    // Just messing around.  Seeing if I can make the cairo surface a sub
    // rectangle of the window, at x=w/8,y=h/4, width=w/2 and height=h/2
    surface = cairo_image_surface_create_for_data(
            (unsigned char *) (pixels + w/8 + (h/4)*stride),
            CAIRO_FORMAT_ARGB32, w/2, h/2, stride*4);
    cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.9, 0.0, 0.7, 0.4);
    cairo_paint(cr);
    cairo_destroy(cr);

    // Again
    surface = cairo_image_surface_create_for_data(
            (unsigned char *) (pixels + w/4 + (h/8)*stride),
            CAIRO_FORMAT_ARGB32, w/3, h/2, stride*4);
    cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.7, 0.8);
    cairo_paint(cr);
    cairo_destroy(cr);

    // Again again ...
    surface = cairo_image_surface_create_for_data(
            (unsigned char *) (pixels + w/5 + (h/20)*stride),
            CAIRO_FORMAT_ARGB32, w/3, h/20, stride*4);
    cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);
    cairo_destroy(cr);

    // So we can control the cairo drawing on sub rectangles in the
    // windows.  A widget can be a sub rectangles in the windows.
    // The widget will only know that it has a surface with a given
    // width and height.
    //
    // If we choose to we can let the widget have a start pixel, width,
    // height, and stride; and let the widget draw on that how ever it
    // wants to.

    // Make a another translucent rectangular hole.  This time without
    // cairo.
    for(uint32_t *pix = pixels + 8*h*stride/10; 
            pix < pixels + 9*h*stride/10; pix += stride)
        for(uint32_t *p = pix + w/3; p < pix + 2*w/3; ++p)
            *p = 0x209090B0; // a r g b
 
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


    // If we make it here it does not seem to crash.
    DSPEW("DONE");

    // We need to make sure that libslate.so is not using any
    // part of libcairo.so.  slDisplay_destroy(d) will cleanup
    // its use of libcairo.so, if any exists.
    //
    // TODO: Maybe add a function like the global slate destructor, for
    // the case when users have many slate displays.
    //
    slDisplay_destroy(d);

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
    // At this time, libslate.so should not be using libcairo.so either.
    //
    cairo_debug_reset_static_data();

    return 0;
}
