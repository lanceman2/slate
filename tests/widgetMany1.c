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
int draw(struct SlWidget *widget, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride) {

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            (unsigned char *) pixels, CAIRO_FORMAT_ARGB32,
            w, h, stride);

    cairo_t *cr = cairo_create(surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0, 0.9, 0.9, 0.8);
    cairo_paint(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return SlDrawReturn_configure;
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
            0, 0, 100, 10, 0/*draw()*/, 0,
            0);
    if(!win) return 1; // fail

    slWidget_create((void *) win, 300, 100,
            SlGravity_None/*SlGravity_None => non-container*/,
            SlGreed_None,
            0x00F0F000/* ARGB background color*/,
            0/* borderWidth*/,
            draw, 0, true/*showing*/);

    slWidget_create((void *) win, 200, 100,
            SlGravity_None/*SlGravity_None => non-container*/,
            SlGreed_None,
            0x00000000/* ARGB background color*/,
            0/* borderWidth*/,
            0/*draw()*/, 0, true/*showing*/);

    struct SlWidget *w = slWidget_create((void *) win, 0, 0,
            SlGravity_LR/*SlGravity_None => non-container*/,
            SlGreed_None,
            0xF0FF0000/* ARGB background color*/,
            6/* borderWidth*/,
            0/*draw()*/, 0, true/*showing*/);

    for(int i=0; i<23; ++i)
        slWidget_create((void *) w, 80, 50,
            SlGravity_None/*SlGravity_None => non-container*/,
            SlGreed_None,
            0x2000FF00 + i*10/* ARGB background color*/,
            0/* borderWidth*/,
            0/*draw()*/, 0, true/*showing*/);


    slWindow_show(win, true/*dispatch*/);

    fprintf(stderr, "\nPress Key <Alt-F4> on the new window to exit\n\n");

#ifdef LOOP
    while(slDisplay_dispatch(d));
#endif

    slDisplay_destroy(d);

    cairo_debug_reset_static_data();

    return 0;
}
