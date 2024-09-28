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

        parent->lastChild->nextSibling = widget;
    } else {
        DASSERT(!parent->lastChild);

        parent->firstChild = widget;
    }
    widget->prevSibling = parent->lastChild;
    parent->lastChild = widget;
}


static inline void
RemoveFromSurfaceList(struct SlSurface *parent, struct SlSurface *widget) {


}


void DestroyWidget(struct SlWidget *widget) {

    DASSERT(widget);

    struct SlSurface *parent = widget->surface.parent;
    DASSERT(parent);

    RemoveFromSurfaceList(parent, &widget->surface);

    DZMEM(widget, sizeof(*widget));
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

    // Add to the surface list.
    AddToSurfaceList(parent, &widget->surface);

    return widget;
}

