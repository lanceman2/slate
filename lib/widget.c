#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "widget.h"

struct SlSurface *slWidget_getSurface(struct SlWidget *widget) {

    ASSERT(widget->surface.type == SlSurfaceType_widget);
    return &widget->surface;
}


// Add widget as the lastChild in parent.
//
static inline void
AddToSurfaceList(struct SlSurface *parent, struct SlSurface *widget) {

    DASSERT(parent);
    DASSERT(widget);

    // This function assumes that the widget links are all 0; so we are
    // adding a new surface to a list in parent as the lastChild.
    DASSERT(widget->parent == 0);
    DASSERT(widget->firstChild == 0);
    DASSERT(widget->lastChild == 0);
    DASSERT(widget->nextSibling == 0);
    DASSERT(widget->prevSibling == 0);

    widget->parent = parent;

    if(parent->firstChild) {
        DASSERT(parent->lastChild);
        DASSERT(parent->lastChild->nextSibling == 0);
        DASSERT(parent->firstChild->prevSibling == 0);
        widget->prevSibling = parent->lastChild;
        parent->lastChild->nextSibling = widget;
    } else {
        DASSERT(!parent->lastChild);
        parent->firstChild = widget;
    }
    parent->lastChild = widget;
}

// Remove widget from it's list which has the parent "parent".
//
static inline void
RemoveFromSurfaceList(struct SlSurface *parent, struct SlSurface *widget) {

    DASSERT(parent);
    DASSERT(widget);
    DASSERT(parent == widget->parent);
    DASSERT(parent->firstChild);
    DASSERT(parent->lastChild);

    if(widget->nextSibling) {
        DASSERT(parent->lastChild != widget);
        widget->nextSibling->prevSibling = widget->prevSibling;
    } else {
        DASSERT(parent->lastChild == widget);
        parent->lastChild = widget->prevSibling;
    }

    if(widget->prevSibling) {
        DASSERT(parent->firstChild != widget);
        widget->prevSibling->nextSibling = widget->nextSibling;
        widget->prevSibling = 0;
    } else {
        DASSERT(parent->firstChild == widget);
        parent->firstChild = widget->nextSibling;
    }

    widget->nextSibling = 0;
    widget->parent = 0;
}


// This function may recurse (call itself).
//
void DestroyWidget(struct SlSurface *surface) {

    DASSERT(surface);
    DASSERT(surface->type == SlSurfaceType_widget);

    // Destroy Children
    while(surface->lastChild)
        DestroyWidget(surface->lastChild);

    RemoveFromSurfaceList(surface->parent, surface);

    struct SlWidget *w = (struct SlWidget *) (
        ((uint8_t *) surface) - offsetof(struct SlWidget, surface));

    DZMEM(w, sizeof(*w));
    free(w);
}

// See declaration of slWidget_create() in ../include/slate.h for more
// information in the comments.
//
struct SlWidget *slWidget_create(
        struct SlSurface *parent,
        uint32_t width, uint32_t height,
        enum SlGravity gravity,
        enum SlGreed greed,
        uint32_t backgroundColor, // A R G B with one byte for each one.
        uint32_t borderWidth, // part of the container surface between
                              // children
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
                uint32_t w, uint32_t h, uint32_t stride),
        bool hide) {

    ASSERT(parent);

    struct SlWidget *widget = calloc(1, sizeof(*widget));
    ASSERT(widget, "calloc(1,%zu) failed", sizeof(*widget));

    widget->surface.type = SlSurfaceType_widget;
    widget->surface.gravity = gravity;
    widget->surface.draw = draw;
    widget->surface.width = width;
    widget->surface.height = height;
    widget->surface.stride = parent->stride;
    widget->surface.backgroundColor = backgroundColor;
    widget->surface.borderWidth = borderWidth;
    widget->surface.draw = draw;
    widget->greed = greed;

    // Add to the surface list.
    AddToSurfaceList(parent, &widget->surface);

    // NOTES:  Need to have a user API/mechanism to hold the window from
    // drawing (or doing like things) until "all" widgets (users choose
    // what all is) are added to the windows' tree of widgets.   ....
    //
    //  How about a slWindow_compose(win)  ????
    //


    // MORE CODE HERE ..........




    return widget;
}

