#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"



struct wl_shm *shm = 0;
struct wl_compositor *compositor = 0;
struct xdg_wm_base *xdg_wm_base = 0;


// We set this to the SlWindow with the wl_surface that got the
// pointer enter and unset it on pointer leave:
static struct SlWindow *currentWindow = 0;


// wl_display is a singleton object and so are many other wayland-client
// objects.  That's one point against Wayland.
//
// I guess that means there is zero or one global instances in existence
// for a given process at a given time.  I need to test if it's the
// library destructor that brings the number of them to zero and/or does
// the wl_display_disconnect() actually work correctly; or do both the
// library destructor and the wl_display_disconnect() work.  Let's hope it
// does not leak system resources like other singletons do; like
// QApplication and gtk_init().  I'm not optimistic about other peoples
// code.  Rightly so, given I guessed that QApplication and gtk_init()
// leaked system resources (file descriptors and memory mappings), and
// later found that they do.  Now I just assume everyone writes shitty
// code; I know I do.
//
// Test codes run with Valgrind show wl_display_disconnect() actually
// works correctly.  Hurray for libwayland-client.so not sucking!

// returned from wl_display_connect().
struct wl_display *wl_display = 0;

static struct wl_registry *wl_registry = 0;
static struct wl_seat *wl_seat = 0;
static struct wl_pointer *pointer = 0;
static struct wl_keyboard *kb = 0;

static uint32_t handle_global_error;


// Protect the processes list of SlDisplay structs
// in struct SlDisplay::firstWindow,lastWindow
//
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// Count the number of successful slDisplay_create() calls
// that are not paired with slDisplay_destroy().
static uint32_t displayCount = 0;


static void xdg_wm_base_handle_ping(void *data,
		struct xdg_wm_base *xdg_wm_base, uint32_t serial) {

    DSPEW("PING");
    // The compositor will send us a ping event to check that we're
    // responsive.  We need to send back a pong request immediately.
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping
};


static void enter(void *data,
        struct wl_pointer *wl_pointer, uint32_t serial,
        struct wl_surface *wl_surface, wl_fixed_t x,  wl_fixed_t y) {

    DASSERT(!currentWindow);
    currentWindow = wl_surface_get_user_data(wl_surface);
    DSPEW("wl_surface=%p x,y=%d,%d",wl_surface, x, y);
}

static void leave(void *data, struct wl_pointer *wl_pointer,
        uint32_t serial, struct wl_surface *wl_surface) {
    
    DASSERT(currentWindow);
    DASSERT((void *) currentWindow == wl_surface_get_user_data(wl_surface));
    currentWindow = 0;

    DSPEW();
}

static void motion(void *, struct wl_pointer *, uint32_t,
        wl_fixed_t,  wl_fixed_t) {
    //DSPEW();
}

static void button(void *, struct wl_pointer *wl_pointer,
        uint32_t serial,
        uint32_t time,
        uint32_t button, uint32_t state) {
    DSPEW();


}

static void axis(void *, struct wl_pointer *, uint32_t,
        uint32_t,  wl_fixed_t) {
    DSPEW();
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = enter,
    .leave = leave,
    .motion = motion,
    .button = button,
    .axis = axis
};

static void kb_map(void* data, struct wl_keyboard* kb,
        uint32_t frmt, int32_t fd, uint32_t sz) {
    DSPEW();
}

static void kb_enter(void* data, struct wl_keyboard* kb,
        uint32_t ser, struct wl_surface* srfc,
        struct wl_array* keys) {
    DSPEW();
}

static void kb_leave(void* data, struct wl_keyboard* kb,
        uint32_t ser, struct wl_surface* srfc) {
    DSPEW();
}

static void kb_key(void* data, struct wl_keyboard* kb,
        uint32_t ser, uint32_t t, uint32_t key,
        uint32_t stat) {

    DSPEW("key=%" PRIu32, key);
}

static void kb_mod(void* data, struct wl_keyboard* kb,
        uint32_t ser, uint32_t dep, uint32_t lat,
        uint32_t lock, uint32_t grp) {
    DSPEW();
}

static void kb_repeat(void* data, struct wl_keyboard* kb,
        int32_t rate, int32_t del) {
    DSPEW();
}

static struct wl_keyboard_listener kb_listener = {
    .keymap = kb_map,
    .enter = kb_enter,
    .leave = kb_leave,
    .key = kb_key,
    .modifiers = kb_mod,
    .repeat_info = kb_repeat
};


static void seat_handle_capabilities(void *data, struct wl_seat *seat,
		uint32_t capabilities) {
    // If the wl_seat has the pointer capability, start listening to pointer
    // events
    DASSERT(seat == wl_seat);
    DASSERT(!pointer);
    DASSERT(!kb);
    DASSERT(capabilities & WL_SEAT_CAPABILITY_POINTER);

    if(capabilities & WL_SEAT_CAPABILITY_POINTER) {
        pointer = wl_seat_get_pointer(seat);
        DASSERT(pointer);
	wl_pointer_add_listener(pointer, &pointer_listener, seat);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD && !kb) {
	kb = wl_seat_get_keyboard(seat);
        DASSERT(kb);
	wl_keyboard_add_listener(kb, &kb_listener, 0);
    }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
};


static void handle_global(void *data, struct wl_registry *registry,
            uint32_t name, const char *interface, uint32_t version) {

    DASSERT(wl_registry == registry);

    //DSPEW("name=\"%s\" (%" PRIu32 ")", wl_shm_interface.name, name);

    if(!strcmp(interface, wl_shm_interface.name)) {
	shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
        DASSERT(shm);
        if(!shm) {
            ERROR("wl_registry_bind(,,) for shm failed");
            handle_global_error = 1;
        }
    } else if(!strcmp(interface, wl_seat_interface.name)) {
        // I'm guessing we can only get one wayland seat.
        DASSERT(wl_seat == 0);
        wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        if(!wl_seat) {
            ERROR("wl_registry_bind(,,) for seat failed");
            handle_global_error = 2;
            return;
        }
	if(wl_seat_add_listener(wl_seat, &seat_listener, NULL)) {
            ERROR("wl_seat_add_listener() failed");
            handle_global_error = 3;
        }
    } else if(!strcmp(interface, wl_compositor_interface.name)) {
        compositor = wl_registry_bind(registry, name,
                &wl_compositor_interface, 1);
        if(!compositor) {
            ERROR("wl_registry_bind(,,) for compositor failed");
            handle_global_error = 4;
        }
    } else if(!strcmp(interface, xdg_wm_base_interface.name)) {
	xdg_wm_base = wl_registry_bind(registry, name,
                &xdg_wm_base_interface, 1);
        if(!xdg_wm_base) {
            ERROR("wl_registry_bind(,,) for xdg_wm_base failed");
            handle_global_error = 5;
            return;
        }
	if(xdg_wm_base_add_listener(xdg_wm_base,
                    &xdg_wm_base_listener, 0)) {
            ERROR("xdg_wm_base_add_listener(,,) failed");
            handle_global_error = 6;
        }
    }
}


static void handle_global_remove(void *data,
        struct wl_registry *registry, uint32_t name) {
    // I care to see this.  Never seen it yet...
    ERROR("called for name=%" PRIu32, name);
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};


// The ends of the list of displays:
static struct SlDisplay *firstDisplay = 0, *lastDisplay = 0;


static inline void CleanupProcess(void) {

    DASSERT(wl_display);

    if(kb) {
        DASSERT(wl_seat);
        // We leak memory if we don't call this.
        wl_keyboard_release(kb);
        kb = 0;
    }

    if(pointer) {
        DASSERT(wl_seat);
        // We leak memory if we don't call this.
        wl_pointer_release(pointer);
        pointer = 0;
    }

    if(wl_seat) {
        // cleanup seat
        //
        // What do ya know; Valgrind showed me that not calling this
        // leaked memory.  I'm so happy that libwayland-client.so
        // cleans up after itself.  Life is good.
        wl_seat_release(wl_seat);
        wl_seat = 0;
    }

    if(shm) {
        // Valgrind shows this to cleanup.
        wl_shm_destroy(shm);
        shm = 0;
    }

    if(compositor) {
        // Valgrind shows this to cleanup.
        wl_compositor_destroy(compositor);
        compositor = 0;
    }

    if(xdg_wm_base) {
        // Valgrind shows this to cleanup.
        xdg_wm_base_destroy(xdg_wm_base);
        xdg_wm_base = 0;
    }

    // valgrind shows this to cleanup something:
    if(wl_registry) {
        wl_registry_destroy(wl_registry);
        wl_registry = 0;
    }
    // A valgrind test shows that this cleans up.  Removing the
    // wl_display_disconnect() below makes the valgrind tests fail.
    //
    //   ../tests/030_display.c and ../tests/032_displayN.c
    //
    wl_display_disconnect(wl_display);
    wl_display = 0;
}

// Return true on error and false on success.
//
static inline bool CheckDisplay(void) {

    if(wl_display)
        // We got it already.
        return false;

    DASSERT(!displayCount);
    wl_display = wl_display_connect(0);
    if(!wl_display) {
        ERROR("wl_display_connect() failed");
        return true;
    }
    // I think that there is only one of these registry things per
    // process too (like wl_display).  We do not need a copy of it
    // after this call given that we are given it back to us in the
    // listener callback functions.
    wl_registry = wl_display_get_registry(wl_display);
    if(!wl_registry) {
        ERROR("wl_display_get_registry(%p) failed", wl_display);
        return true;
    }
    if(wl_registry_add_listener(wl_registry, &registry_listener, 0)) {
        ERROR("wl_registry_add_listener(%p, %p,) failed",
                wl_registry, &registry_listener);
        CleanupProcess();
        return true;
    }
    handle_global_error = 0;

    // I'm guessing this can block.
    if(wl_display_roundtrip(wl_display) == -1 || handle_global_error) {
        ERROR("wl_display_roundtrip(%p) failed handle_global_error=%"
                PRIu32, wl_display, handle_global_error);
        // TODO: I see no way to get an error value out of handle_global()
        // so I added variable handle_global_error.
        CleanupProcess();
        return true;
    }
    if(!shm || !compositor || !xdg_wm_base) {
	ERROR("no wl_shm, wl_compositor or xdg_wm_base support");
        CleanupProcess();
        return true;
    }

    return false; // success
}


struct SlDisplay *slDisplay_create(void) {

    struct SlDisplay *d = 0;

    CHECK(pthread_mutex_lock(&mutex));

    if(CheckDisplay())
        goto finish;

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

    DSPEW();

    // Destroy all slate toplevel windows that are owned by this display (d).
    while(d->lastToplevel) {
        DASSERT(d->lastToplevel->window.type == SlWindowType_topLevel);
        DASSERT(d->lastToplevel->window.parent == 0);
        _slWindow_destroy(d, &d->lastToplevel->window);
    }

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

    if(displayCount == 0)
        CleanupProcess();

    CHECK(pthread_mutex_unlock(&mutex));
}


#if 0 // We may need this.
static void __attribute__((constructor)) create(void) {

    DSPEW();
}
#endif


static void __attribute__((destructor)) destructor(void) {

    if(getenv("SLATE_NO_CLEANUP"))
        // TODO: I'm not sure if this is a good idea.
        // But, I can test when the slDisplay_destroy() works and does not
        // work correctly.
        return;

    DSPEW("libslate.so cleaning up");


    // This will cleanup after a sloppy user of this API.
    // 
    while(lastDisplay)
        slDisplay_destroy(lastDisplay);

    DSPEW("libslate.so done");
}


bool slDisplay_dispatch(struct SlDisplay *d) {

    if(d->done)
        return false;

    if(wl_display_dispatch(wl_display) != -1) {
        return true;
    }

    return false;
}
