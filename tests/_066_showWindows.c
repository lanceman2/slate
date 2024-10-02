#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();

    const uint32_t NUM_WINS = 4;

    struct SlWindow *w[NUM_WINS];

    for(int i=0; i<NUM_WINS; ++i) {
        w[i] = slWindow_createToplevel(d, 100, 100, i*100, 10, 0, true);
        if(!w[i]) return 1; // error fail
    }

    fprintf(stderr, "\nType <alt-F4> to exit\n\n");

    while(slDisplay_dispatch(d));

#ifdef CLEANUP
    // This will get done in the libslate.so destructor any way, but
    // we still test for this slWindow_destroy() here.
    for(int i=0; i<NUM_WINS; ++i)
        slWindow_destroy(w[i]);
#endif
    // Let the libslate.so destructor cleanup the display and windows
    // if CLEANUP is not defined. 

    return 0;
}
