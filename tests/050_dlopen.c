// This is a better interactive test than a non-interactive test.
//
// Checks if ../lib/libslate.so leaks some system resources and hence
// checks if libwayland-client.so leaks too.  As of Mar 15 2024 it
// looks like this finds no leaks; but keep in mind this test is
// not extensive.
//
// Edit this file, compile, and run different versions of this test.
// First read this code and understand it.  Please do not check-in a
// crap/test edit of this file (especially if it fails).
//
// This program loads ./_050_DSO.so via dlopen(2).

#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>

#include "../lib/debug.h"


static inline void Wait(const char*str) {

#if 1 // Too look at some system resources.
    pid_t pid = getpid();
    const size_t Len = 1024;
    char buf[Len];
    printf("\n---------------%s-----------------\n", str);
    snprintf(buf, Len, "cat /proc/%d/maps > /proc/%d/fd/1", pid, pid);
    system(buf);
    snprintf(buf, Len, "ls -flt --color=yes /proc/%d/fd > /proc/%d/fd/1",
            pid, pid);
    system(buf);
    printf("-------------------------------------\n");
#endif
}


int main(void) {

    // Running this test interactively with the #if 0 and #if 1 changed
    // helps us see what's going on...

#if 0
    // This will make the libslate.so destructor bail before cleaning up.
    // And so we are testing that this code cleans up all the resources
    // created in this file.
    //
    ASSERT(0 == setenv("SLATE_NO_CLEANUP", "1", 1));
#endif

    void *dlh = dlopen("./_050_DSO.so", RTLD_NOW);
    ASSERT(dlh);

    struct SlDisplay *(*makeDisplay)(void) = dlsym(dlh, "makeDisplay");
    ASSERT(makeDisplay);

    struct SlDisplay *d = makeDisplay();

#if 1
    void (*destroyDisplay)(struct SlDisplay *d) = dlsym(dlh, "destroyDisplay");
    ASSERT(destroyDisplay);

    Wait("Before destroyDisplay()");

    destroyDisplay(d);
#else
    ERROR("d=%p", d);
#endif

    // Not calling slDisplay_destroy():
    //
    // We'll be leaking memory if the libslate.so destructor does not
    // cleanup.  So testing this with and without valgrind gives different
    // results.

    // This will show if libslate.so and libwayland-client.so get
    // unmapped.  Note: running with valgrind makes more files.
    //
    Wait("Before dlclose()");

    //sleep(1);
 
    ASSERT(0 == dlclose(dlh));

    // This will show if libslate.so and libwayland-client.so get
    // unmapped and if there's extra files left open.

    Wait("After dlclose()");

    return 0;
}
