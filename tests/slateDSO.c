#include <stdlib.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

int dummy = 0;


struct SlDisplay *makeDisplay(void) {

    WARN();
    return slDisplay_create();
}

void destroyDisplay(struct SlDisplay *d) {

    WARN();
    slDisplay_destroy(d);
}

static const uint32_t NUM_WINS = 10; 

struct SlDisplay *makeDisplayAndWindows(void) {

    WARN();

    struct SlDisplay *d = slDisplay_create();

    for(int i=0; i<NUM_WINS; ++i)
        slWindow_createTop(d, 10, 10, 0, 0, 0);

    return d;
}

