#include <stdbool.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


int main(void) {

    struct SlDisplay *d = slDisplay_create();

    for(int i=0; i<10; ++i)
        slWindow_create(d);

    // Let the libslate.so destructor cleanup the display and windows.

    return 0;
}
