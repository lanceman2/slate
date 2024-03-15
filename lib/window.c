#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

#include "../include/slate.h"

#include "debug.h"
#include "window.h"


struct SlWindow *slWindow_create(void) {

    struct SlWindow *w = calloc(1, sizeof(*w));
    ASSERT(w, "calloc(1,%zu) failed", sizeof(*w));

    return w;
}

void slWindow_destroy(struct SlWindow *w) {

    DZMEM(w, sizeof(*w));
    free(w);
}
