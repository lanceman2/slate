#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *w = slWindow_createToplevel(
            d, 300, 300, 10, 10, 0);
    if(!w)
        return 1; // fail

    struct SlWindow *p = slWindow_createPopup(
            w, 100, 100, 10, 10, 0);
    if(!p) return 1; // fail


    return 0;
}
