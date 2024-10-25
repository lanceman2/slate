//  example:
//  run:  ./valgrind_run_tests _061_window_noCleanup

// This test runs interactively, so we start the file name with "_" and so
// it's not run with the default test programs the are run with the bash
// script ./run_tests .

#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; //fail
    struct SlWindow *w = slWindow_createToplevel(d, 100, 100, 0, 0,
            0/*draw*/, 0, SL_SHOWING);
    if(!w) return 1; // fail

    fprintf(stderr, "\n\nHIT <Alt-F4> to exit\n\n");


    while(slDisplay_dispatch(d));

#ifdef CLEANUP
    slWindow_destroy(w);
    slDisplay_destroy(d);
 // else the libslate.so library destructor will cleanup
 // and valgrind tests show it does.
#endif

    return 0;
}
