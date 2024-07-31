#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"
#include "shm.h"


static void xdg_surface_handle_configure(struct SlWindow *win,
	    struct xdg_surface *xdg_surface, uint32_t serial) {

    // The compositor configures our surface, acknowledge the configure
    // event
    xdg_surface_ack_configure(win->xdg_surface, serial);

    if(win->configured)
        // If this isn't the first configure event we've received, we
        // already have a buffer attached, so no need to do anything.
        // Commit the surface to apply the configure acknowledgement.
	wl_surface_commit(win->wl_surface);

    win->configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = (void *) xdg_surface_handle_configure,
};

static void noop(void) {

    DSPEW();
}

static void xdg_toplevel_handle_close(struct SlWindow *win,
		struct xdg_toplevel *xdg_toplevel) {

    win->open = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = (void *) noop,
	.close = (void *) xdg_toplevel_handle_close,
};


void _slWindow_destroy(struct SlDisplay *d,
        struct SlWindow *win) {

    DASSERT(d);
    DASSERT(win);
    DASSERT(win->display == d);


    // Cleanup wayland stuff for this window:

    // Cleanup in reverse order of construction.
    
    if(win->buffer) {
        wl_buffer_destroy(win->buffer);
        win->buffer = 0;
    }

    if(win->xdg_toplevel)
        xdg_toplevel_destroy(win->xdg_toplevel);

    if(win->xdg_surface)
        xdg_surface_destroy(win->xdg_surface);

    if(win->wl_surface)
        wl_surface_destroy(win->wl_surface);





    // Remove win from the display windows list:
    if(win->next) {
        DASSERT(win != d->lastWindow);
        win->next->prev = win->prev;
    } else {
        DASSERT(win == d->lastWindow);
        d->lastWindow = win->prev;
    }
    if(win->prev) {
        DASSERT(win != d->firstWindow);
        win->prev->next = win->next;
    } else {
        DASSERT(win == d->firstWindow);
        d->firstWindow = win->next;
    }

    DZMEM(win, sizeof(*win));
    free(win);
}


static bool create_buffer(struct SlWindow *win) {

    DASSERT(win->width > 0);
    DASSERT(win->height > 0);

    int stride = win->width * 4;
    int size = stride * win->height;

    // Allocate a shared memory file with the right size
    int fd = create_shm_file(size);

    if(fd < 0)
	return true;

    // Map the shared memory file
    void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shm_data == MAP_FAILED) {
        ERROR("mmap() failed");
	close(fd);
	return true;
    }

    // Create a wl_buffer from our shared memory file descriptor
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    if(!pool) {
        ERROR("wl_shm_create_pool() failed");
        close(fd);
        return true;
    }


    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
            win->width, win->height,
            stride, WL_SHM_FORMAT_ARGB8888);
    if(!pool) {
        ERROR("wl_shm_pool_create_buffer() failed");
        wl_shm_pool_destroy(pool);
        close(fd);
        return true;
    }

    wl_shm_pool_destroy(pool);

    // Now that we've mapped the file and created the wl_buffer, we no longer
    // need to keep file descriptor opened
    close(fd);

    memset(shm_data, 0, size);

    win->shm_data = shm_data;
    win->buffer = buffer;

    return false;
}

struct SlWindow *slWindow_createTop(struct SlDisplay *d,
        uint32_t w, uint32_t h, int32_t x, int32_t y) {

    ASSERT(d);

    // If the user is destroying the display (SlDisplay) in another thread
    // while calling this in this thread we are screwed.  I guess it's up
    // to the user to make the construction and destruction of the display
    // thread-safe.
    DASSERT(wl_display);
    DASSERT(shm);
    DASSERT(compositor);
    DASSERT(xdg_wm_base);

    struct SlWindow *win = calloc(1, sizeof(*win));
    ASSERT(win, "calloc(1,%zu) failed", sizeof(*win));

    win->width = w;
    win->height = h;

    // TODO:
    // Lets not code for funny width and heights yet. Uncommon values
    // could be used in the future.
    ASSERT(w > 0);
    ASSERT(h > 0);

    CHECK(pthread_mutex_lock(&d->mutex));

    // Add win to the displays windows list:
    if(d->lastWindow) {
        DASSERT(d->firstWindow);
        DASSERT(!d->lastWindow->next);
        DASSERT(!d->firstWindow->prev);
        d->lastWindow->next = win;
        win->prev = d->lastWindow;
    } else {
        DASSERT(!d->firstWindow);
        d->firstWindow = win;
    }
    d->lastWindow = win;

    win->display = d;

    // Wayland window stuff:

    win->wl_surface = wl_compositor_create_surface(compositor);
    if(!win->wl_surface) {
        ERROR("wl_compositor_create_surface() failed");
        goto fail;
    }

    win->xdg_surface = xdg_wm_base_get_xdg_surface(
            xdg_wm_base, win->wl_surface);
    if(!win->xdg_surface) {
        ERROR("xdg_wm_base_get_xdg_surface() failed");
        goto fail;
    }

    win->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
    if(!win->xdg_toplevel) {
        ERROR("xdg_surface_get_toplevel() failed");
        goto fail;
    }

    if(xdg_surface_add_listener(win->xdg_surface,
                &xdg_surface_listener, win)) {
        ERROR("xdg_surface_add_listener(,,) failed");
        goto fail;
    }
    if(xdg_toplevel_add_listener(win->xdg_toplevel,
                &xdg_toplevel_listener, 0)) {
        ERROR("xdg_toplevel_add_listener(,,) failed");
        goto fail;
    }

    // Perform the initial commit and wait for the first configure event
    wl_surface_commit(win->wl_surface);

    while(!win->configured)
        if(wl_display_dispatch(wl_display) == -1) {
	    ERROR("wl_display_dispatch() failed can't configure window");
            goto fail;
        }

    if(create_buffer(win))
        goto fail;

    // Success:

    CHECK(pthread_mutex_unlock(&d->mutex));
    return win;

fail:

    _slWindow_destroy(d, win);
    CHECK(pthread_mutex_unlock(&d->mutex));
    return 0; // failure.
}


void slWindow_destroy(struct SlWindow *w) {

    struct SlDisplay *d = w->display;

    CHECK(pthread_mutex_lock(&d->mutex));
    _slWindow_destroy(d, w);
    CHECK(pthread_mutex_unlock(&d->mutex));
}
