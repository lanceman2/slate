#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"


// wl_display is a singleton object.  That's one point against Wayland.
//
// I guess that means there is zero or one global instances in existence
// for a given process at a given time.  I need to test if it's the
// library destructor that brings the number of them to zero or does the
// wl_display_disconnect() actually work correctly; or do both the
// library destructor and the wl_display_disconnect() work.  Let's hope it
// does not leak system resources like other singletons do; like
// QApplication and gtk_init().  I'm not optimistic about other peoples
// code.  Rightly so, given I guessed that QApplication and gtk_init()
// leaked system resources (file descriptors and memory mappings), and
// later found that they do.  Now I just assume everyone writes shitty
// code; I know I do.
//
static struct wl_display *wl_display = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Count the number of successful slDisplay_create() calls
// that are not paired with slDisplay_destroy().
static uint32_t displayCount = 0;

// The ends of the list of displays:
static struct SlDisplay *firstDisplay = 0, *lastDisplay = 0;


struct SlDisplay *slDisplay_create(void) {

    struct SlDisplay *d = 0;

    CHECK(pthread_mutex_lock(&mutex));

    if(!wl_display) {
        DASSERT(!displayCount);
        wl_display = wl_display_connect(0);
        if(!wl_display) {
            ERROR("wl_display_connect() failed");
            goto finish;
        }
    }

    d = calloc(1, sizeof(*d));
    ASSERT(d, "calloc(1,%zu) failed", sizeof(*d));

    // Add this display (d) to the last in the list of displays:
    if(lastDisplay) {
        DASSERT(firstDisplay);
        DASSERT(!lastDisplay->next);
        DASSERT(!firstDisplay->prev);
        lastDisplay->next = d;
        d->prev = lastDisplay;
    } else {
        DASSERT(!firstDisplay);
        firstDisplay = d;
    }
    lastDisplay = d;

    ++displayCount;

finish:

    CHECK(pthread_mutex_unlock(&mutex));
    CHECK(pthread_mutex_init(&d->mutex, 0));

    return d;
}


void slDisplay_destroy(struct SlDisplay *d) {

    DASSERT(d);

    CHECK(pthread_mutex_lock(&d->mutex));

    // Destroy all slate windows that are owned by this display (d).
    //
    // There may be just one wayland display, but we have many slate
    // displays that own other data/function things like slate windows.  I
    // knew I made this slate display abstraction for some reason.  A
    // program can have many modules that have a display (or displays)
    // that do not necessarily know about each other (other modules).  And
    // the displays in each module can have any number of slate windows.
    // It's this idea of modular coding, putting together bits and pieces
    // code that do not necessarily know about each other.
    while(d->lastWindow) _slWindow_destroy(d, d->lastWindow);

    CHECK(pthread_mutex_unlock(&d->mutex));

    CHECK(pthread_mutex_destroy(&d->mutex));

    CHECK(pthread_mutex_lock(&mutex));

    DASSERT(wl_display);
    DASSERT(displayCount);

    --displayCount;

    DASSERT(firstDisplay);
    DASSERT(lastDisplay);

    // Remove this display (d) from the list of displays:
    if(d->next) {
        DASSERT(d != lastDisplay);
        d->next->prev = d->prev;
    } else {
        DASSERT(d == lastDisplay);
        lastDisplay = d->prev;
    }
    if(d->prev) {
        DASSERT(d != firstDisplay);
        d->prev->next = d->next;
    } else {
        DASSERT(d == firstDisplay);
        firstDisplay = d->next;
    }

    DZMEM(d, sizeof(*d));
    free(d);

    if(displayCount == 0) {
        // A valgrind test shows that this cleans up.  Removing the
        // wl_display_disconnect below makes the valgrind tests fail.
        //
        //   ../tests/030_display.c and ../tests/032_displayN.c
        //
        wl_display_disconnect(wl_display);
        wl_display = 0;
    }

    CHECK(pthread_mutex_unlock(&mutex));
}


#if 0 // We may need this.
static void __attribute__((constructor)) create(void) {

    DSPEW();
}
#endif


static void __attribute__((destructor)) destroy(void) {

    if(getenv("SLATE_NO_CLEANUP"))
        // TODO: I'm not sure if this is a good idea.
        // But, I can test when the slDisplay_destroy() works and does not
        // work correctly.
        return;

    DSPEW();

    // This will cleanup after a sloppy user of this API.
    // 
    while(lastDisplay)
        slDisplay_destroy(lastDisplay);
}


bool slDisplay_dispatch(struct SlDisplay *d) {

    return false;
}
