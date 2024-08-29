#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"
#include "shm.h"

// Popup windows (surfaces) get/have a pointer focus grab as part of the
// wayland protocol.  Popups are special/temporary.

// From: https://wayland-client-d.dpldocs.info/wayland.client.protocol.wl_shell_surface_set_popup.html
//
// DOC QUOTE: A popup surface is a transient surface with an added pointer
// grab.
//
// DOC QUOTE: The x and y arguments specify the location of the upper left
// corner of the surface relative to the upper left corner of the parent
// surface, in surface-local coordinates.



struct SlWindow *slWindow_createPopup(struct SlWindow *parent,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, void *pixels,
            uint32_t w, uint32_t h, uint32_t stride)) {

    DASSERT(xdg_wm_base);
    DASSERT(parent);
    ASSERT(parent->type == SlWindowType_topLevel);
    struct SlToplevel *t = (void *) parent;
    struct SlDisplay *d = t->display;
    DASSERT(d);

    struct SlPopup *p = calloc(1, sizeof(*p));
    ASSERT(p, "calloc(1,%zu) failed", sizeof(*p));

    struct SlWindow *win = &p->window;
    p->parent = (void *) parent;
    win->type = SlWindowType_popup;


    CHECK(pthread_mutex_lock(&d->mutex));

    // Start with the generic wayland surface stuff.
    if(CreateWindow(d, win, w, h, x, y, draw))
        goto fail;

    // Add this popup window to the list of children in the
    // parent toplevel window.
    AddChild(t, win);

    // Add stuff specific to the popup surface/window thingy.
    p->xdg_positioner = xdg_wm_base_create_positioner(xdg_wm_base);
    if(!p->xdg_positioner) {
        ERROR("xdg_wm_base_create_positioner() failed");
        goto fail;
    }
    p->xdg_popup = xdg_surface_get_popup(win->xdg_surface, parent->xdg_surface,
                    p->xdg_positioner);
    if(!p->xdg_popup) {
        ERROR("xdg_surface_get_popup() failed");
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
