#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"



void _slWindow_destroy(struct SlDisplay *d,
        struct SlWindow *w) {

    DASSERT(d);
    DASSERT(w);
    DASSERT(w->display == d);


    // Cleanup wayland stuff for this window:
   
    if(w->wl_surface)
        wl_surface_destroy(w->wl_surface);





    // Remove w from the display windows list:
    if(w->next) {
        DASSERT(w != d->lastWindow);
        w->next->prev = w->prev;
    } else {
        DASSERT(w == d->lastWindow);
        d->lastWindow = w->prev;
    }
    if(w->prev) {
        DASSERT(w != d->firstWindow);
        w->prev->next = w->next;
    } else {
        DASSERT(w == d->firstWindow);
        d->firstWindow = w->next;
    }

    DZMEM(w, sizeof(*w));
    free(w);
}


struct SlWindow *slWindow_createTop(struct SlDisplay *d,
        uint32_t w, uint32_t h, int32_t x, int32_t y) {

    ASSERT(d);

    // If the user is destroying the display (SlDisplay) in another thread
    // while calling this in this thread we are screwed.  I guess it's up
    // to the user to make the construction and destruction of the display
    // thread-safe.
    DASSERT(shm);
    DASSERT(compositor);
    DASSERT(xdg_wm_base);

    struct SlWindow *win = calloc(1, sizeof(*win));
    ASSERT(win, "calloc(1,%zu) failed", sizeof(*win));

    CHECK(pthread_mutex_lock(&d->mutex));

    // Add win to the displays windows list:
    if(d->lastWindow) {
        DASSERT(d->firstWindow);
        DASSERT(!d->lastWindow->next);
        DASSERT(!d->firstWindow->prev);
        d->lastWindow->next = win;
        win->prev = d->lastWindow;
    } else {
        DASSERT(!d->firstWindow);
        d->firstWindow = win;
    }
    d->lastWindow = win;

    win->display = d;

    // Wayland window stuff:

    win->wl_surface = wl_compositor_create_surface(compositor);
    if(!win->wl_surface) {
        ERROR("wl_compositor_create_surface() failed");
        goto fail;
    }


    // Success:

    CHECK(pthread_mutex_unlock(&d->mutex));
    return win;

fail:

    _slWindow_destroy(d, win);
    CHECK(pthread_mutex_unlock(&d->mutex));
    return 0; // failure.
}


void slWindow_destroy(struct SlWindow *w) {

    struct SlDisplay *d = w->display;

    CHECK(pthread_mutex_lock(&d->mutex));
    _slWindow_destroy(d, w);
    CHECK(pthread_mutex_unlock(&d->mutex));
}
