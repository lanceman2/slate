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
#include "widget.h"
#include "display.h"
#include "shm.h"
#include "../include/slate_drawingUtils.h"



static bool AddFrameListener(struct SlWindow *win);

static
void (*surface_damage_func)(struct wl_surface *wl_surface,
        int32_t x, int32_t y, int32_t width, int32_t height) = 0;


static inline void PostDrawDamage(struct SlWindow *win) {

    struct wl_surface *wl_surface = win->wl_surface;

    wl_surface_attach(wl_surface, win->wl_buffer, 0, 0);

    // For newer version
    // wl_surface_damage_buffer(wl_surface, 0, 0, INT32_MAX, INT32_MAX);
    //
    // For older wayland client version 1
    // wl_surface_damage(wl_surface, 0, 0, INT32_MAX, INT32_MAX);
    //
    surface_damage_func(wl_surface, 0, 0, INT32_MAX, INT32_MAX);

    wl_surface_commit(wl_surface);
}


void PushPixels(struct SlWindow *win) {

    PostDrawDamage(win);
}


// The default used when the callback is not set by the API user.
static
void GetChildrenPosition(struct SlWindow *win,
            uint32_t width, uint32_t height,
            uint32_t childrenWidth, uint32_t childrenHeight,
            uint32_t *childrenX, uint32_t *childrenY) {

    *childrenX = 0;
    *childrenY = 0;
}


// In case we need to cleanup this Frame DrawQueue thing before it gets
// used.
//
// This is a case of editing this queue list that only happens when a
// widget (or window) is destroyed.
//
static inline
void RemoveDrawFrameQueue(struct DrawQueue *dq, struct SlSurface *s) {

    DASSERT(dq);
    DASSERT(s);

    if(!s->queued) return;
 
    struct SlSurface *prev = 0;
    for(struct SlSurface *sf = dq->first; sf; sf = sf->next) {

        if(sf == s) {
            if(prev) {
                DASSERT(sf != dq->first);
                prev->next = sf->next;
            } else {
                DASSERT(sf == dq->first);
                dq->first = s->next;
            }

            if(s->next) {
                DASSERT(sf != dq->last);
                s->next = 0;
            } else {
                DASSERT(sf == dq->last);
                dq->last = prev;
            }
            break;
        }

        prev = sf;
    }
}


void RemoveDrawFrameQueues(struct SlWindow *win, struct SlSurface *s) {

    DASSERT(win);
    DASSERT(s);
    
    if(!s->queued) return;

    RemoveDrawFrameQueue(&win->dq1, s);
    RemoveDrawFrameQueue(&win->dq2, s);

    if(!win->dq1.first && !win->dq2.first) {
        if(win->wl_callback) {
            wl_callback_destroy(win->wl_callback);
            win->wl_callback = 0;
        }
    }
}


static inline
void AddDrawFrameQueue(struct SlWindow *win, struct SlSurface *s) {

    DASSERT(win);
    DASSERT(s);
    DASSERT(!s->next);
    DASSERT(s->draw);

    struct DrawQueue *dq = win->writingDQ;
    DASSERT(dq);
    DASSERT(win->readingDQ);
    DASSERT(dq != win->readingDQ);

    // TODO: Is it possible that it is in the queue already?
    if(s->queued) {
        DASSERT(0, "this happened");
        return;
    }

    DASSERT(!s->next);
    // Add this, s, to the last in the queue, win->drawFrameLast.
    //
    if(dq->last) {
        DASSERT(dq->first);
        DASSERT(!dq->last->next);
        dq->last->next = s;
    } else {
        DASSERT(!dq->first);
        dq->first = s;
    }
    dq->last = s;

    s->queued = true;
}


static inline
struct SlSurface *PopDrawFrameQueue(struct SlWindow *win) {

    DASSERT(win);
    struct DrawQueue *dq = win->readingDQ;
    DASSERT(dq);
    DASSERT(win->writingDQ);
    DASSERT(dq != win->writingDQ);

    struct SlSurface *s = dq->first;

    if(s) {
        DASSERT(dq->last);
        dq->first = s->next;
        if(s == dq->last) {
            DASSERT(!s->next);
            dq->last = 0;
        } else {
            DASSERT(s->next);
            s->next = 0;
        }
        DASSERT(s->queued);
        s->queued = false;
    } else {
        DASSERT(!dq->last);
    }

    return s;
}


static inline void DrawSurface(struct SlWindow *win,
        struct SlSurface *s, uint32_t *pixels, uint32_t stride) {

    DASSERT(s);
    DASSERT(pixels);

    // The starting pixel for this widget (or window) surface:
    pixels += s->allocation.x + s->allocation.y * stride / 4;

    // Note: we do not draw the background color if there is a user draw()
    // function callback.  The user that makes a draw() function can draw
    // the background in one function call, if they choose to.

    if(s->draw) {
        // Call the libslate.so API users draw() callback
        int drawReturn = s->draw(s,
                pixels,
                s->allocation.width,
                s->allocation.height,
                stride/*stride in bytes*/);

        switch(drawReturn) {

            case SlDrawReturn_frame:
                // We will continue to call this draw in this frame thingy
                // when the time for the next frame happens.
                //
                if(!win->wl_callback)
                    // TODO: This can fail, so ...
                    AddFrameListener(win);
                AddDrawFrameQueue(win, s);
                break;

            case SlDrawReturn_configure:
        }
    } else {
        sl_drawFilledRectangle(pixels/*starting pixel*/,
            0/*x*/, 0/*y*/,
            s->allocation.width,
            s->allocation.height,
            stride, s->backgroundColor);
    }
}


// This function calls itself.
//
static void DrawSurfaceRecursive(struct SlWindow *win,
        struct SlSurface *s, uint32_t *pixels, uint32_t stride) {

    DrawSurface(win, s, pixels, stride);

    if(s->showingChildren) {
        DASSERT(s->firstChild);
        DASSERT(s->lastChild);
        for(struct SlSurface *sf = s->firstChild; sf; sf = sf->nextSibling)
            if(sf->showing)
                DrawSurfaceRecursive(win, sf, pixels, stride);
    }
}


// Draw all the window's widgets.
//
static inline void DrawAll(struct SlWindow *win) {

    DASSERT(win);
    DASSERT(win->wl_surface);
    DASSERT(win->wl_buffer);
    DASSERT(win->surface.allocation.width);
    DASSERT(win->surface.allocation.height);
    DASSERT(win->pixels);
    DASSERT(win->stride);
    DASSERT(win->sharedBufferSize);
    DASSERT(win->wl_callback == 0);
    DASSERT(win->surface.showing);

    // We better not draw if we are in the middle of setting up the shared
    // memory; and yes we do setup surface.allocation.width,height some
    // time before we make the shared memory pixel buffers.
    DASSERT(win->surface.allocation.height *
            win->stride == win->sharedBufferSize);

    // Draw all children widgets too.
    //
    DrawSurfaceRecursive(win, &win->surface, win->pixels, win->stride);
}


// This does nothing if there is no buffer stuff yet.
//
void FreeBuffer(struct SlWindow *win) {

    DASSERT(win);

    if(win->wl_buffer) {
        wl_buffer_destroy(win->wl_buffer);
        win->wl_buffer = 0;
    }

    if(win->pixels) {

        DASSERT(win->sharedBufferSize);

        if(munmap(win->pixels,
                    win->sharedBufferSize)) {
            ERROR("munmap(%p,%zu) failed",
                    win->pixels,
                    win->sharedBufferSize);
            // TODO: What can we do about this failure???
            DASSERT(0, "munmap(%p,%zu) failed",
                    win->pixels,
                    win->sharedBufferSize);
        }
        win->pixels = 0;
    }
}


// Each SlWindow does this once and only once.
//
static inline
bool WaitForConfigureXDGSurface(struct SlWindow *win) {

    DASSERT(win);
    DASSERT(win->wl_surface);
    DASSERT(win->xdg_surface);

    for(uint32_t loopCount = 0; !win->xdg_configured;) {
        // We are wanting for the compositor (server) to send an event
        // that says we have a valid xdg_surface.  No size, but just the
        // xdg_surface.
        //
        // TODO: Could this get tricky if there are lots of other events
        // that are not related to this surface/window?  Or could it...
        // Thread-safety?
        //
        if(wl_display_dispatch(wl_display) == -1) {
	    ERROR("wl_display_dispatch() failed can't configure window");
            return true;
        }
        // Make it robust.
        ASSERT(++loopCount < 10000, "Failed to get xdg_surface "
                "configure event in %" PRIu32 " loop tries", loopCount);
    }
    return false; // false ==> success
}


// This creates struct SlWindow:: pixels, and buffer; and also
// recreates.
//
// See also FreeBuffer()
//
static inline bool RecreateBuffer(struct SlWindow *win) {

    DASSERT(win);
    DASSERT(!win->needAllocate);
    DASSERT(win->surface.allocation.width);
    DASSERT(win->surface.allocation.height);
    DASSERT(win->surface.allocation.width * SLATE_PIXEL_SIZE ==
            win->stride);
    DASSERT(win->sharedBufferSize);

    // Given the name Recreate we need to cleanup the old stuff.
    //
    // This does nothing if there is no wl_buffer and stuff yet.
    FreeBuffer(win);

    // We will get these here:
    DASSERT(!win->pixels);
    DASSERT(!win->wl_buffer);
    DASSERT(!win->surface.allocation.x);
    DASSERT(!win->surface.allocation.y);

    int stride = win->stride;
    // in bytes
    size_t size = win->sharedBufferSize;
    uint32_t *pixels;

    int fd = create_shm_file(size);

    if(fd < 0)
        // create_shm_file() should have spewed already.
	return true;

    // Map the (pixels) shared memory file to this processes virtual
    // address space.
    pixels = mmap(0, size,
            PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(pixels == MAP_FAILED) {
        ERROR("mmap(0, size=%zu,PROT_READ|PROT_WRITE,"
                "MAP_SHARED,fd=%d,0) failed", size, fd);
	close(fd);
	return true;
    }

    // Create a wl_buffer from our shared memory file descriptor.
    //
    // Here we pass the file descriptor to the compositor process (or at
    // least get ready to pass it) via a UNIX domain socket.  The
    // compositor process should take care of removing (via unlink(2)) the
    // file after the inter-process shared pixels memory is mapped in the
    // compositor process.
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    if(!pool) {
        ERROR("wl_shm_create_pool() failed");
        // TODO: What if this fails?:
        ASSERT(munmap(pixels, size) == 0);
        close(fd);
        return true;
    }

    win->wl_buffer = wl_shm_pool_create_buffer(pool, 0,
            win->surface.allocation.width,
            win->surface.allocation.height,
            stride, WL_SHM_FORMAT_ARGB8888);
    if(!win->wl_buffer) {
        ERROR("wl_shm_pool_create_buffer() failed");
        wl_shm_pool_destroy(pool);
        // TODO: What if this fails?:
        ASSERT(munmap(pixels, size) == 0);
        close(fd);
        return true;
    }

    // I declare that we officially have an allocation for this slate
    // window surface.
    //
    // We define it this way for x,y allocations in slate window (at least
    // for toplevel).
    //
    // TODO: What about slate non-toplevel windows?
    //
    // Kind of stupid at this point, but the allocation::width,height can
    // set now to the user requested size win::surface::width,height.
    //
    // We need to keep the user requested size separate from the allocated
    // size, in case a resize event comes from the compositor to change
    // the allocated size, making the allocated size different from the
    // API user requested size.
    win->pixels = pixels;

    // We have what we needed.  Inter-process shared memory at process
    // virtual address win->surface.allocation.pixels.
    //
    // We need to consider if it is worth it to keep the pool around
    // and use wl_shm_pool_resize(), or to just recreate it at each
    // resize.  We just do not know if creating this pool thingy costs us
    // a system call or what...  Need to look at the libwayland-client.so
    // code.  Okay: shm_pool_unref() in src/wayland-sh.c shows that the
    // wl_shm_pool thingy has a memory mapping and a file descriptor in
    // it, and I see a call to mremap(2), so ya maybe we should keep this
    // shm_pool thingy around between resizing.
    //
    // TODO: keep this pool in struct SlWindow and destroy it with the
    // SlWindow window.  I expect that this does not necessarily free all
    // the shm_pool resources, but just decrements a counter and there's
    // a reference to it in the wl_buffer that owns it after this destory.
    //
    wl_shm_pool_destroy(pool);

    // Now that we've mapped the file and created the wl_buffer, we no
    // longer need to keep file descriptor opened.
    close(fd);

    // Start with a some known value for the pixel memory.  The value of
    // all zeros is not visible on my screen.
    sl_drawFilledRectangle(pixels/*surface starting pixel*/,
            0/*x*/, 0/*y*/,
            win->surface.allocation.width,
            win->surface.allocation.height, stride,
            win->surface.backgroundColor);

    return false;
}


static void xdg_surface_handle_configure(struct SlWindow *win,
	    struct xdg_surface *xdg_surface, uint32_t serial) {

    DASSERT(win);
    DASSERT(win->xdg_surface);
    DASSERT(win->xdg_surface == xdg_surface);
    DASSERT(!win->needAllocate);
    DASSERT(win->wl_buffer);
    DASSERT(win->pixels);
    DASSERT(win->surface.allocation.width);
    DASSERT(win->surface.allocation.height);
    DASSERT(win->sharedBufferSize);

    if(win->xdg_configured)
        INFO("Got %" PRIu32 " xdg_surface_configure events serial=%"
                PRIu32 " for window=%p",  win->xdg_configured, serial,
                win);

    // TODO: I'm not sure how to use this event.
    //
    // Looks like mouse pointer enter can make these events, but I also
    // get a separate pointer enter event too.

    // Count these events until we figure out what the hell they are for.
    ++win->xdg_configured;

    // Note: we are not worried about this win->xdg_configured counter
    // wrapping back to 0, we just miss one INFO() printing.

    // The compositor (server) configures our xdg_surface. Acknowledge the
    // configure event.
    //
    // I'm guessing they are for whatever we decide, but it looks like we
    // must acknowledge the first one that this window gets.
    xdg_surface_ack_configure(win->xdg_surface, serial);


    DSPEW("Got first xdg_surface_configure event for window=%p", win);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = (void *) xdg_surface_handle_configure,
};

static void toplevel_configure(void) {

    DSPEW();
}

static void xdg_toplevel_handle_close(struct SlToplevel *t,
		struct xdg_toplevel *xdg_toplevel) {

    DASSERT(t);
    DASSERT(xdg_toplevel);
    DASSERT(xdg_toplevel == t->xdg_toplevel);

    DSPEW();
    t->window.open = false;
    // TODO: Looks like if any top level window gets here the "display" is
    // done.  My testing on KDE plasma with wayland (2024 Jul 31), hit key
    // press Alt-<F4> seems to be a destroy the "display connection"
    // thingy and not just the particular window that was in focus for the
    // Alt-<F4> event.  I note the if I do not destroy the display in time
    // (fast enough) the process is terminated.  It may be kwin server
    // signaling my program (TODO: I need to check that).  Seems like a
    // race condition in the kwin server/client interaction/protocol.
    //
    t->display->done = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = (void *) toplevel_configure,
	.close = (void *) xdg_toplevel_handle_close,
};


static inline void FreeToplevel(struct SlDisplay *d, struct SlToplevel *t) {

    DASSERT(d);
    DASSERT(d == t->display);
    DASSERT(t->window.surface.type == SlSurfaceType_topLevel);
    DASSERT(t->window.parent == 0);

    if(t->xdg_toplevel)
        xdg_toplevel_destroy(t->xdg_toplevel);

    // Remove t from the slate display slate toplevel windows list:
    if(t->next) {
        DASSERT(t != d->lastToplevel);
        t->next->prev = t->prev;
    } else {
        DASSERT(t == d->lastToplevel);
        d->lastToplevel = t->prev;
    }
    if(t->prev) {
        DASSERT(t != d->firstToplevel);
        t->prev->next = t->next;
    } else {
        DASSERT(t == d->firstToplevel);
        d->firstToplevel = t->next;
    }

    memset(t, 0, sizeof(*t));
    free(t);
}


void _slWindow_destroy(struct SlDisplay *d,
        struct SlWindow *win) {

    DASSERT(d);
    DASSERT(win);

    // Cleanup the list of widgets in
    // SlSurface::firstChild,lastChild,nextSibling,prevSibling
    //
    while(win->surface.lastChild)
        DestroyWidget(win->surface.lastChild);

    RemoveDrawFrameQueues(win, &win->surface);

    // Cleanup wayland stuff for this window in reverse order of
    // construction.  (So I think...)

    switch(win->surface.type) {

        case SlSurfaceType_topLevel:
            break;
        case SlSurfaceType_popup:
        {
            struct SlPopup *p = (void *) win;
            if(p->xdg_popup) {
                xdg_popup_destroy(p->xdg_popup);
                p->xdg_popup = 0;
            }
            if(p->xdg_positioner) {
                xdg_positioner_destroy(p->xdg_positioner);
                p->xdg_positioner = 0;
            }
            break;
        }
        default:
            ASSERT(0, "WRITE CODE to Free window type %d",
                    win->surface.type);
    }


    // Free the shared memory pixels buffer.
    FreeBuffer(win);

    if(win->wl_callback)
        wl_callback_destroy(win->wl_callback);

    if(win->xdg_surface)
        xdg_surface_destroy(win->xdg_surface);

    if(win->wl_surface)
        wl_surface_destroy(win->wl_surface);

    //DSPEW("Cleaning up win->surface.type=%d", win->surface.type);

    switch(win->surface.type) {

        case SlSurfaceType_topLevel:
            while(((struct SlToplevel *)win)->lastChild) {
                DASSERT(((struct SlToplevel *)win
                            )->lastChild->surface.type ==
                        SlSurfaceType_popup);
                _slWindow_destroy(d, ((struct SlToplevel *)win)->lastChild);
            }
            FreeToplevel(d, (void *) win);
            break;
        case SlSurfaceType_popup:
            RemoveChild(((struct SlPopup *)win)->parent, win);
            memset(win, 0, sizeof(struct SlPopup));
            free(win);
            break;
        default:
            ASSERT(0, "WRITE CODE to Free window surface type %d",
                    win->surface.type);
    }
}


static struct wl_callback_listener frame_listener;


static bool AddFrameListener(struct SlWindow *win) {

    DASSERT(win);
    DASSERT(!win->wl_callback);

    win->wl_callback = wl_surface_frame(win->wl_surface);
    if(!win->wl_callback) {
        ERROR("wl_surface_frame() failed");
        return true; // failure
    }
    if(wl_callback_add_listener(win->wl_callback,
                    &frame_listener, win)) {
        ERROR("wl_callback_add_listener() failed");
        wl_callback_destroy(win->wl_callback);
        win->wl_callback = 0;
        return true; // failure
    }

    return false; // return false == success
}


static void frame_new(struct SlWindow *win,
        struct wl_callback* cb, uint32_t a) {\

    DASSERT(win);
    DASSERT(win->wl_surface);
    DASSERT(cb);
    DASSERT(cb == win->wl_callback);
    DASSERT(win->surface.allocation.width > 0);
    DASSERT(win->surface.allocation.height > 0);

    // TODO: extra line of code not needed after the first frame_new()
    // call.
    if(!win->framed) win->framed = true;


    // Switch the draw queues, the one we read and the one we write.
    if(win->writingDQ == &win->dq1) {
        DASSERT(win->readingDQ == &win->dq2);
        win->writingDQ = &win->dq2;
        win->readingDQ = &win->dq1;
    } else {
        DASSERT(win->readingDQ == &win->dq1);
        DASSERT(win->writingDQ == &win->dq2);
        win->writingDQ = &win->dq1;
        win->readingDQ = &win->dq2;
    }

    // Why is this not automatic?  It seems this must be done every call
    // to the wl_callback (this function); so why is this not automatic?
    wl_callback_destroy(cb);
    // This keeps this code consistent.  Mark that we used the wl_callback
    // and so it's done for this for this time.
    win->wl_callback = 0;

    // It could happen that a widget and its surface is destroyed along
    // with the frame request, before this call.
    bool gotOne = false;

    for(struct SlSurface *s = PopDrawFrameQueue(win); s;
            s = PopDrawFrameQueue(win)) {
        // We are assuming that the order of the surface draws will be
        // such that parents do not draw over their children.
        // This should be so given that these frame draw requests come
        // in the order of the sequence of draw callbacks.
        DrawSurface(win, s, win->pixels, win->stride);
        if(!gotOne)
            gotOne = true;
    }

    // The reading draw queue should be empty.
    DASSERT(!win->readingDQ->first);

    if(gotOne)
        PostDrawDamage(win);
}


static
struct wl_callback_listener frame_listener = {
    .done = (void (*)(void* data, struct wl_callback* cb, uint32_t a))
        frame_new
};


// TODO: There has got to be a better way:
//
// I'm a little lost here...  This is somewhat of a not so nice hack
// function.  Maybe there is a better way to do this.
//
static inline void GetSurfaceDamageFunction(struct SlWindow *win) {

    DASSERT(win);
    DASSERT(win->wl_surface);

    if(!surface_damage_func) {
        // WTF (what the fuck): Why do they change the names of functions
        // in an API, and still keep both accessible even when one is
        // broken.
        //
        // This may not be setting the correct function needed to see what
        // surface_damage_func should be set to.  We just make it work on
        // the current system (the deprecated function
        // wl_surface_damage()).  When this fails you'll see (from the
        // stderr tty spew):
        // 
        //  wl_display@1: error 1: invalid method 9 (since 1 < 4), object wl_surface@3
        //
        // Or something like that.
        //
        // Given it took me a month (at least) to find this (not so great
        // fix) we're putting lots of comment-age here.
        //
        // This one may be correct. We would have hoped that
        // wl_proxy_get_version() would have used argument prototype const
        // struct wl_proxy * so we do not change memory at/in
        // win->wl_surface, but alas they do not:
        uint32_t version = wl_proxy_get_version(
                (struct wl_proxy *) win->wl_surface);
        //
        // These two may be the wrong function to use (I leave here for
        // the record):
        //uint32_t version = xdg_toplevel_get_version(win->xdg_toplevel);
        //uint32_t version = xdg_surface_get_version(win->xdg_surface);

        switch(version) {
            case 1:
                // Older deprecated version (see:
                // https://wayland-book.com/surfaces-in-depth/damaging-surfaces.html)
                DSPEW("Using deprecated function wl_surface_damage()"
                        " version=%" PRIu32, version);
                surface_damage_func = wl_surface_damage;
                break;

            case 4: // We saw a version 4 in another compiled program that used
                    // wl_surface_damage_buffer()
            default:
                // newer version:
                DSPEW("Using newer function wl_surface_damage_buffer() "
                        "version=%" PRIu32, version);
                surface_damage_func = wl_surface_damage_buffer;
        }
    }
}


static inline void AddToplevel(struct SlDisplay *d,
        struct SlToplevel *t) {

    DASSERT(d);
    DASSERT(t->display = d);
    DASSERT(t->window.surface.type == SlSurfaceType_topLevel);
    DASSERT(t->window.parent == 0);

    // Add t to the displays toplevel windows list:
    if(d->lastToplevel) {
        DASSERT(d->firstToplevel);
        DASSERT(!d->lastToplevel->next);
        DASSERT(!d->firstToplevel->prev);
        d->lastToplevel->next = t;
        t->prev = d->lastToplevel;
    } else {
        DASSERT(!d->firstToplevel);
        d->firstToplevel = t;
    }
    d->lastToplevel = t;
}


// Creates the generic SlWindow before it's made into a particular
// slate surface type of window like toplevel, popup, sub, fullscreen (???).
//
// Return true on error.
bool CreateWindow(struct SlDisplay *d, struct SlWindow *win,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        void (*getChildrenPosition)(struct SlWindow *win,
            uint32_t width, uint32_t height,
            uint32_t childrenWidth, uint32_t childrenHeight,
            uint32_t *childrenX, uint32_t *childrenY)
        ) {

    ASSERT(d);
    DASSERT(win);
    DASSERT(!win->wl_surface);
    DASSERT(!win->xdg_surface);

    // If the user is destroying the display (SlDisplay) in another thread
    // while calling this in this thread we are screwed.  I guess it's up
    // to the user to make the construction and destruction of the display
    // thread-safe.
    DASSERT(wl_display);
    // shm, compositor, and xdg_wm_base are a process singlet global
    // structures.  We keep them in the SlDisplay code (display.c).
    DASSERT(shm);
    DASSERT(compositor);
    DASSERT(xdg_wm_base);

    // TODO:
    // Lets not code for funny width and heights yet. Uncommon values
    // could be used in the future.
    //ASSERT(w > 0);
    //ASSERT(h > 0);

    win->writingDQ = &win->dq1;
    win->readingDQ = &win->dq2;

    // Extra width and height if win acts as a widget container.
    // Clearly these are not set in slWindow_createToplevel() and
    // like functions.
    win->surface.width = 0;
    win->surface.height = 0;

    if(!getChildrenPosition)
        win->surface.getChildrenPosition = (void *) GetChildrenPosition;
    else
        win->surface.getChildrenPosition = (void *) getChildrenPosition;

    // If win has no widget children, then the window is just a API users
    // drawing area with this width and height.
    //
    // If there are widget children then this width and height are a user
    // suggested window size at the first displaying of the window.
    win->width = w;
    win->height = h;

    // x,y are used for popup windows.
    win->x = x;
    win->y = y;

    win->surface.draw = (void *) draw;
    // default window background color
    win->surface.backgroundColor = 0x70FF00F0;
    // default window borderWidth
    win->surface.borderWidth = 4;
    win->needAllocate = true;

    win->wl_surface = wl_compositor_create_surface(compositor);
    if(!win->wl_surface) {
        ERROR("wl_compositor_create_surface() failed");
        return true;
    }

    wl_surface_set_user_data(win->wl_surface, win);

    GetSurfaceDamageFunction(win);

    // https://wayland-book.com/xdg-shell-basics.html
    //
    // The XDG (cross-desktop group) shell is a standard protocol
    // extension for Wayland which describes the semantics for application
    // windows. 
    win->xdg_surface = xdg_wm_base_get_xdg_surface(
            xdg_wm_base, win->wl_surface);

    if(!win->xdg_surface) {
        ERROR("xdg_wm_base_get_xdg_surface() failed");
        return true;
    }

    if(xdg_surface_add_listener(win->xdg_surface,
                &xdg_surface_listener, win)) {
        ERROR("xdg_surface_add_listener(,,) failed");
        return true;
    }

    DASSERT(!win->wl_buffer);
    DASSERT(win->wl_surface);
    DASSERT(!win->pixels);
    DASSERT(!win->wl_callback);

    return false; // false ==success
}


// TODO: Thread safety?
//
void slWindow_setDraw(struct SlWindow *win,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride)) {
    DASSERT(win);
    DASSERT(win->wl_surface);
    DASSERT(win->xdg_configured);
    DASSERT(win->surface.type == SlSurfaceType_topLevel,
            "WRITE MORE CODE HERE");

    win->surface.draw = (void *) draw;

    if(win->wl_buffer) {
        DrawAll(win);
        PostDrawDamage(win);
    }
}


bool ShowSurface(struct SlWindow *win, bool dispatch) {

    DASSERT(win);
    DASSERT(win->wl_surface);
    DASSERT(win->xdg_surface);

    bool needAllocate = win->needAllocate;

    if(win->needAllocate)
        slWindow_compose(win);

    if(!win->wl_buffer || needAllocate) {
        DASSERT(!win->wl_buffer);
        DASSERT(!win->pixels);

        // This is the first call to RecreateBuffer() for this
        // xdg_surface.   RecreateBuffer() may get called again from a
        // window resize from the desktop (mouse/pointer resize event
        // thingy, or other desktop interaction like a menu thingy).
        if(RecreateBuffer(win)) {
            // TODO: What to do for this error case?
            DASSERT(0);
            return true;
        }
    }

    DASSERT(win->wl_buffer);
    DASSERT(win->pixels);


    // Perform the initial commit and wait for the first configure event
    // for this surface in this slWindow.
    //
    // Removing this, wl_surface_commit(), hangs the process.  This seems
    // to prime the pump.
    //
    wl_surface_commit(win->wl_surface);

    // TODO: Do we need to do this again for a window resize?
    //
    if(!win->xdg_configured && WaitForConfigureXDGSurface(win)) {
        DASSERT(!win->xdg_configured);
        return true;
    }

    DrawAll(win);

    win->framed = false;

    if(!win->wl_callback) {

        // TODO: is this really needed?

        // We just use one frame call unless a surface draw() returns
        // SlDrawReturn_frame, and in that case it would have already
        // called AddFrameListener(win).
        //
        // TODO: This can fail...
        AddFrameListener(win);
    }

    // Force the server to act on the new pixels in this new window.
    PostDrawDamage(win);

    if(!dispatch) return false;

    // We have seen that the window is not visible on the desktop yet (at
    // least not always); at this time.  Why?  We do not know.
    //
    // TODO: We need this new window/surface to be showing on the desktop
    // after this function returns, otherwise things get fucked up; like
    // added popup windows are not shown above the parent toplevel
    // windows.  We are just guessing how to make this robust.
    //
    // We'll now wait for the compositor in the following
    // wl_display_dispatch(); and hope it is visible after the first call
    // to the frame callback function.  If the user did not pass in a
    // draw() callback we just do not setup for another frame callback
    // after the first one.
    //
    while(!win->framed) {
        // TODO: Could this get tricky if there are lots of other
        // events that are not related to this surface/window?
        //
        if(wl_display_dispatch(wl_display) == -1) {
            ERROR("wl_display_dispatch() failed can't show window");
            return true;
        }
    }

    // reset flag:
    win->framed = false;

    return false; // success
}


struct SlWindow *slWindow_createToplevel(struct SlDisplay *d,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        void (*getChildrenPosition)(struct SlWindow *win,
            uint32_t width, uint32_t height,
            uint32_t childrenWidth, uint32_t childrenHeight,
            uint32_t *childrenX, uint32_t *childrenY),
        bool showing) {

    struct SlToplevel *t = calloc(1, sizeof(*t));
    ASSERT(t, "calloc(1,%zu) failed", sizeof(*t));

    struct SlWindow *win = &t->window;
    t->display = d;
    win->surface.type = SlSurfaceType_topLevel;

    // Some defaults:
    win->surface.gravity = SlGravity_TB;
    win->surface.showing = showing;


    CHECK(pthread_mutex_lock(&d->mutex));

    // Add win to the display's toplevel windows list:
    AddToplevel(d, (void *) win);

    // Create the generic wayland surface stuff.
    if(CreateWindow(d, win, w, h, x, y, draw,
                (void *) getChildrenPosition))
        goto fail;

    // Now create wayland toplevel specific stuff.
    //
    t->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
    if(!t->xdg_toplevel) {
        ERROR("xdg_surface_get_toplevel() failed");
        goto fail;
    }
    //
    if(xdg_toplevel_add_listener(t->xdg_toplevel,
                &xdg_toplevel_listener, win)) {
        ERROR("xdg_toplevel_add_listener(,,) failed");
        goto fail;
    }

    // We will call ShowSurface() at a later time if the window is
    // not set as "showing" yet.
    if(showing && ShowSurface(win, true))
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

    DASSERT(w);
    DASSERT(w->surface.type == SlSurfaceType_topLevel, "WRITE MORE CODE");
    struct SlDisplay *d = ((struct SlToplevel *) w)->display;
    DASSERT(d);

    CHECK(pthread_mutex_lock(&d->mutex));
    _slWindow_destroy(d, w);
    CHECK(pthread_mutex_unlock(&d->mutex));
}
