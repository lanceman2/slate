#include <stdlib.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"



int main(void) {

    // This will make the libslate.so destructor bail before
    // cleaning up.  And so we are testing that this code cleans
    // up all the resources created in in this file.
    ASSERT(0 == setenv("SLATE_NO_CLEANUP", "1", 1));

    for(int i=0; i<5; ++i) {
        struct SlDisplay *display = slDisplay_create();
        slDisplay_destroy(display);
    }

    int N = 6;
    struct SlDisplay *display[N];

    // Make a few and remove a few.
    for(int i=0; i<N; ++i)
        display[i] = slDisplay_create();
    for(int i=0; i<N; ++i)
        slDisplay_destroy(display[i]);

    // Again:
    N = 5;
    for(int i=0; i<N; ++i)
        display[i] = slDisplay_create();
    for(int i=0; i<N; ++i)
        slDisplay_destroy(display[i]);

    return 0;
}
