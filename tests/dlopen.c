// This is a better interactive test than a non-interactive test.
//
// Checks if ../lib/libslate.so leaks some system resources and hence
// checks if libwayland-client.so leaks too.  As of Mar 15 2024 it
// looks like this finds no leaks; but keep in mind this test is
// not extensive.
//
// As ../lib/libslate.so evolves, it will be nice to keep this test
// passing.  So many other API libraries, like Qt6 and GTK3 fail this kind
// of test.  I tried to fix Qt6 and GTK3 but their developers are not
// receptive.  Qt6 and GTK3 (likely GTK4 too) are not robust code (by
// design) as I define it.  They say we didn't design it to do that
// (unload the libraries), hence Qt and GTK are not robust by design.  They
// are not even into documenting this defect.
//
// Edit this file, compile, and run different versions of this test.
// First read this code and understand it.  Please do not check-in a
// crap/test edit of this file (especially if it fails).
//
// Note we say the above (checks if libwayland-client.so leaks) assuming
// that this the binary links to libwayland-client.so indirectly through
// ../lib/slateDSO.so; if that changes this a different test needs to be
// added to this suite of tests.

// This program loads ../lib/slateDSO.so via dlopen(2).
// This program calls system(3) which forks and shit.

#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>

#include "../lib/debug.h"



static inline void Wait(const char*str) {

    pid_t pid = getpid();
    const size_t Len = 1024;
    char buf[Len];
    printf("\n---------------%s-----------------\n", str);
    snprintf(buf, Len, "cat /proc/%d/maps > /proc/%d/fd/1", pid, pid);
    printf("------------ files mapped \"%s\"\n", buf);
    // TODO: Looks like the call to system opens the anon_inode:inotify
    // file and uses it in later calls too.  I guess system(3) is kind of
    // a shit libc call anyway.  I do not feel like confirming this...
    system(buf);
    snprintf(buf, Len, "ls -flt --color=yes /proc/%d/fd > /proc/%d/fd/1",
            pid, pid);
    printf("------------ files open \"%s\"\n", buf);
    system(buf);
    printf("-------------------------------------\n\n");
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

    Wait("Before dlopen(\"./slateDSO.so\",)");

    void *dlh = dlopen("./slateDSO.so", RTLD_NOW);
    ASSERT(dlh);

    struct SlDisplay *(*makeDisplay)(void) = dlsym(dlh, FUNC);
    ASSERT(makeDisplay);

    struct SlDisplay *d = makeDisplay();

    void (*destroyDisplay)(struct SlDisplay *d) = dlsym(dlh, "destroyDisplay");
    ASSERT(destroyDisplay);

    Wait("Before destroyDisplay()");

    destroyDisplay(d);

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
