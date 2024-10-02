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





void configure(void *data,
        struct xdg_popup *xdg_popup,
	int32_t x, int32_t y,
	int32_t width, int32_t height) {

    DSPEW();
}

void popup_done(void *data, struct xdg_popup *xdg_popup) {

    DSPEW();
}

void repositioned(void *data,
        struct xdg_popup *xdg_popup,
	uint32_t token) {

    DSPEW();
}


static struct xdg_popup_listener xdg_popup_listener = {

    .configure = configure,
    .popup_done = popup_done,
    .repositioned = repositioned
};


struct SlWindow *slWindow_createPopup(struct SlWindow *parent,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        bool showing) {

    DASSERT(xdg_wm_base);
    DASSERT(parent);
    ASSERT(parent->surface.type == SlSurfaceType_topLevel);
    struct SlToplevel *t = (void *) parent;
    struct SlDisplay *d = t->display;
    DASSERT(d);

    struct SlPopup *p = calloc(1, sizeof(*p));
    ASSERT(p, "calloc(1,%zu) failed", sizeof(*p));

    struct SlWindow *win = &p->window;
    p->parent = (void *) parent;
    win->surface.type = SlSurfaceType_popup;
    win->surface.showing = showing;


    CHECK(pthread_mutex_lock(&d->mutex));

    // Add this popup window to the list of children in the
    // parent toplevel window.
    AddChild(t, win);

    // Start with the generic wayland surface stuff.
    if(CreateWindow(d, win, w, h, x, y, draw))
        goto fail;

    // Add stuff specific to the popup surface/window thingy.
    p->xdg_positioner = xdg_wm_base_create_positioner(xdg_wm_base);
    if(!p->xdg_positioner) {
        ERROR("xdg_wm_base_create_positioner() failed");
        goto fail;
    }

    DASSERT(win->surface.width > 0);
    DASSERT(win->surface.height > 0);

    xdg_positioner_set_size(p->xdg_positioner,
            win->surface.width, win->surface.height);
    xdg_positioner_set_anchor_rect(p->xdg_positioner,
            win->x, win->y, win->surface.width, win->surface.height);

    p->xdg_popup = xdg_surface_get_popup(win->xdg_surface,
            parent->xdg_surface, p->xdg_positioner);
    if(!p->xdg_popup) {
        ERROR("xdg_surface_get_popup() failed");
        goto fail;
    }

    if(xdg_popup_add_listener(p->xdg_popup,
                &xdg_popup_listener, p)) {
        ERROR("xdg_positioner_add_listener(,,) failed");
        goto fail;
    }

    if(showing && ConfigureSurface(win))
        goto fail;

    // Success:

    CHECK(pthread_mutex_unlock(&d->mutex));
    return win;

fail:

    _slWindow_destroy(d, win);
    CHECK(pthread_mutex_unlock(&d->mutex));
    return 0; // failure.
}
