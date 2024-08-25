#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();

    const uint32_t NUM_WINS = 10;

    struct SlWindow *w[NUM_WINS];

    for(int i=0; i<NUM_WINS; ++i) {
        w[i] = slWindow_createTop(d, 10, 10, i*10, 10, 0);
        if(!w[i])
            return 1; // Fail
    }

#ifdef CLEANUP
    // This will get done in the libslate.so destructor any way, but
    // we still test for this slWindow_destroy() here.
    for(int i=0; i<NUM_WINS; ++i)
        slWindow_destroy(w[i]);
#else
    // Let the libslate.so destructor cleanup the display and windows
    // if CLEANUP is not defined. 

    // Fix -Werror=unused-but-set-variable
    fprintf(stderr, "w=%p\n", w);
#endif

    return 0;
}
