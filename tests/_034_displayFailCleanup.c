#include <stdlib.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"



int main(void) {

#ifdef SLATE_NO_CLEANUP
    // This will make the libslate.so destructor bail before
    // cleaning up.  And so we are testing that this code cleans
    // up all the resources created in this file.
    //
    ASSERT(0 == setenv("SLATE_NO_CLEANUP", "1", 1));
#endif

    slDisplay_create();

    // Not calling slDisplay_destroy():
    //
    // We'll be leaking memory if the libslate.so destructor does not
    // cleanup.

    return 0;
}
