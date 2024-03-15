#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "../include/slate.h"

#include "debug.h"
#include "display.h"


// wl_display is a singleton object.  That's one point against Wayland.
//
// I guess that means there is zero or one global instances in existence
// for a given process.  I need to test if it's the library destructor
// that brings the number of them to zero or does the
// wl_display_disconnect() actually work correctly.   Let's hope it does
// not leak system resources like other singletons do; like QApplication
// and gtk_init().  I'm not optimistic about other peoples code.  Rightly
// so, given I guessed that QApplication and gtk_init() leaked system
// resources (file descriptors and memory mappings), and later found that
// they do.  Now I just assume everyone's writes shitty code.
//
static struct wl_display *display = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Count the number of successful slDisplay_create() calls
// that are not paired with slDisplay_destroy().
static uint32_t displayCount = 0;


struct SlDisplay *slDisplay_create(void) {

    struct SlDisplay *d = 0;

    CHECK(pthread_mutex_lock(&mutex));

    if(!display) {
        DASSERT(!displayCount);
        display = wl_display_connect(0);
        if(!display) {
            ERROR("wl_display_connect() failed");
            goto finish;
        }
    }

    d = calloc(1, sizeof(*d));
    ASSERT(d, "calloc(1,%zu) failed", sizeof(*d));

    ++displayCount;

finish:

    CHECK(pthread_mutex_unlock(&mutex));
    return d;
}


void slDisplay_destroy(struct SlDisplay *d) {

    DASSERT(d);

    CHECK(pthread_mutex_lock(&mutex));

    DASSERT(display);
    DASSERT(displayCount);

    --displayCount;

    DZMEM(d, sizeof(*d));
    free(d);

    if(displayCount == 0) {
        // A valgrind test shows that this cleans up:
        //
        //   ../tests/030_display.c and ../tests/032_displayN.c
        //
        wl_display_disconnect(display);
        display = 0;
    }

    CHECK(pthread_mutex_unlock(&mutex));
}
