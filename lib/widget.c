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
#include "widget.h"

struct SlSurface *slWidget_getSurface(struct SlWidget *widget) {

    ASSERT(widget->surface.type == SlSurfaceType_widget);
    return &widget->surface;
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


    return 0;
}

