#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"


struct SlWindow *slWindow_createTop(struct SlDisplay *d,
        uint32_t w, uint32_t h, int32_t x, int32_t y) {

    ASSERT(d);

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

    CHECK(pthread_mutex_unlock(&d->mutex));

    return win;
}

void _slWindow_destroy(struct SlDisplay *d,
        struct SlWindow *w) {

    DASSERT(d);
    DASSERT(w);
    DASSERT(w->display == d);

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

void slWindow_destroy(struct SlWindow *w) {

    struct SlDisplay *d = w->display;

    CHECK(pthread_mutex_lock(&d->mutex));
    _slWindow_destroy(d, w);
    CHECK(pthread_mutex_unlock(&d->mutex));
}
