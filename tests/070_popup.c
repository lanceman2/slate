#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *w = slWindow_createToplevel(
            d, 400, 300, 10, 10, 0, 0, SL_SHOWING);
    if(!w) return 1; // fail

    struct SlWindow *p = slWindow_createPopup(
            w, 100, 200, 10, 10, 0, 0, true/*showing*/);
    if(!p) return 1; // fail

    return 0;
}
