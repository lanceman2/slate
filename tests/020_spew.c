// Testing that a user can use the debug.h spew stuff that
// is compiled into ../lib/libslate.so
//
#include <slate_debug.h>


int main(void) {

    DSPEW();
    INFO();
    NOTICE();
    WARN();
    ERROR();

    return 0;
}

