// This test runs interactively, so we start the file name with "_" and so
// it's not run with the default test programs the are run with the bash
// script ./run_tests .

#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static bool running = true;


int main(void) {

    struct SlDisplay *d = slDisplay_create();
    struct SlWindow *w = slWindow_createTop(d, 10, 10, 0, 0);

    ASSERT(w);

    while(slDisplay_dispatch(d) && running);

#ifdef CLEANUP
    slWindow_destroy(w);
    slDisplay_destroy(d);
 // else the libslate.so library destructor will cleanup
 // and valgrind tests show it does.
#endif

    return 0;
}
