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

