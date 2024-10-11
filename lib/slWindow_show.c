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


struct SlSurface *slWindow_getSurface(struct SlWindow *window) {

    ASSERT(window->surface.type);
    ASSERT(window->surface.type < SlSurfaceType_widget);
    return &window->surface;
}


// TODO: It is questionable whither of not we need to keep this list of
// children in the toplevel window/surface thingy; it could be the wayland
// client API (application programming interface) has an interface we can
// use to access the toplevel's children and popup's parents.
//
// In any case, we know the parent/child relations by how our libslate.so
// API was called, and we add just a little bit of state data to
// libslate.so.
//
void AddChild(struct SlToplevel *t, struct SlWindow *win) {

    DASSERT(t);
    DASSERT(win);
    DASSERT(t->window.surface.type == SlSurfaceType_topLevel);
    DASSERT(win->surface.type == SlSurfaceType_popup);
    DASSERT(!win->prev);
    DASSERT(!win->next);

    // Add win as the lastChild
    win->prev = t->lastChild;

    if(t->lastChild) {
        DASSERT(t->firstChild);
        DASSERT(!t->firstChild->prev);
        DASSERT(!t->lastChild->next);
        t->lastChild->next = win;
    } else {
        DASSERT(!t->firstChild);
        t->firstChild = win;
    }
    t->lastChild = win;
}


void RemoveChild(struct SlToplevel *t, struct SlWindow *win) {

    DASSERT(t);
    DASSERT(win);
    DASSERT(t->window.surface.type == SlSurfaceType_topLevel);

    if(win->prev) {
        DASSERT(win != t->firstChild);
        win->prev->next = win->next;
    } else {
        DASSERT(win == t->firstChild);
        t->firstChild = win->next;
    }
    if(win->next) {
        DASSERT(win != t->lastChild);
        win->next->prev = win->prev;
        win->next = 0;
    } else {
        DASSERT(win == t->lastChild);
        t->lastChild = win->prev;
    }
    win->prev = 0;
}


// This function calls itself.
//
// Kind-of a requested size based on all children and parent border
// widths.
//
// We do not overwrite the SlSurface::showing. That is set somehow by the
// API user.  Hence we needed the function parameter argument
// parentShowing.
//
// We traverse the widget tree via function recursion.  SlSurface is a
// widget (or window for the first to call).
//
void
AddSizeOfSurface(struct SlSurface *s, bool parentShowing) {

    DASSERT(s);

    // Initialize this, s->showingChildren, now.  It may not be correct
    // now but we will change it soon.
    //
    if(s->firstChild)
        // showingChildren may get set again later in this function:
        s->showingChildren = s->showing;
    else
        // There are no children, so no children are showing.
        s->showingChildren = false;

    if(!s->firstChild) {
        DASSERT(!s->lastChild);
        // This is a leaf node.
        //
        if(s->showing && parentShowing) {
            // We start by giving it what it requests.
            //
            // This surface may be one that could have children but does
            // not yet.
            //
            s->allocation.width  = s->width;
            s->allocation.height = s->height;
        } else {
            // No space for this surface, s.
            s->allocation.width  = 0;
            s->allocation.height = 0;
        }
        return;
    }


    // This is a parent node.
    DASSERT(s->lastChild);

    // See if this parent surface, s, has any showing children; otherwise
    // this surface (and all sub-children) will not be visually present,
    // or get a width/height allocation.
    for(struct SlSurface *sf = s->firstChild; sf; sf = sf->nextSibling) {
        if(sf->showing && !s->showingChildren)
            // We have at least one visible child.
            s->showingChildren = true;
        // Call stack dive toward the child, sf.
        AddSizeOfSurface(sf, s->showing && parentShowing);
    }

    // We now have the size (width,height) of all the children of surface,
    // s.

    if(!s->showing) {
        s->allocation.width  = 0;
        s->allocation.height = 0;
        return;
    }

    if(!s->showingChildren) {
        // We still give an empty container space if it requests it.
        s->allocation.width  = s->width;
        s->allocation.height = s->height;
        return;
    }

    // Now this surface, s, has all its children's allocations tallied.
    // Time to tally this surfaces allocation (width and height) for
    // a container that is confirmed to have visible children.

    // Tally width,height based on children requested width,height and
    // parent border widths.
    //
    uint32_t *width  = &(s->allocation.width);
    uint32_t *height = &(s->allocation.height);

    // Slate container surfaces (widgets and windows) may request space
    // for themselves in addition to space for their children.  A common
    // case is to use the non-child space for decorations that go around
    // the children.
    //
    *width  = s->width;
    *height = s->height;

    // I repeat: Now we know this surface, s, has a least one showing
    // child.

    uint32_t borderWidth = s->borderWidth;

    switch(s->gravity) {

        case SlGravity_TB:// top to bottom
        case SlGravity_BT:// bottom to top

            for(struct SlSurface *sf = s->firstChild; sf;
                    sf = sf->nextSibling) 
                if(sf->showing) {
                    // widget height plus a border
                    *height += (sf->allocation.height + borderWidth);
                    if(*width < sf->allocation.width)
                        *width = sf->allocation.width;
                }
            // border on the top (or bottom)
            *height += borderWidth;
            // Now we have the height additions.

            // For the width we have two borders, the left and the
            // right.
            *width += 2 * borderWidth;
            break;

        case SlGravity_LR:// left to right
        case SlGravity_RL:// right to left

            for(struct SlSurface *sf = s->firstChild; sf;
                    sf = sf->nextSibling) 
                if(sf->showing) {
                    // widget width plus a border
                    *width += (sf->allocation.width + borderWidth);
                    if(*height < sf->allocation.height)
                        *height = sf->allocation.height;
                }
            // border on an end.
            *width += borderWidth;
            // Now we have the width additions.

            // For the height we have two borders, the top and the
            // bottom.
            *height += 2 * borderWidth;
            break;

        case SlGravity_One:
            // By definition there should be one child for a surface with
            // SlGravity_One
            ASSERT(s->firstChild == s->lastChild);
            ASSERT(s->firstChild->showing);

            *width  += (s->firstChild->allocation.width  + 2 * borderWidth);
            *height += (s->firstChild->allocation.height + 2 * borderWidth);
            break;

        case SlGravity_None:
            // We should have excluded this case.
            ASSERT(0, "A surface with SlGravity_None has children WTF");
            break;

        default:
            // Just a dumb-ass code check...
            ASSERT(0, "Write more code here for gravity=%d", s->gravity);
    }
}


static void inline
ShrinkAllocatedWidth(struct SlSurface *s, uint32_t max) {

    DASSERT(s);
    DASSERT(max);

WARN("NEED MORE CODE HERE");

    // The s->allocation.width will be zero if s not showing.
    if(s->allocation.width > max) {
        DASSERT(s->showing);

        // Shrink this surface, s.
        if(s->firstChild && s->showingChildren) {
            // This is a parent node.
            DASSERT(s->lastChild);

            switch(s->gravity) {

                case SlGravity_TB:// top to bottom
                    break;
                case SlGravity_BT:// bottom to top
                    break;
                case SlGravity_LR:// left to right
                    break;
                case SlGravity_RL:// right to left
                    break;
                case SlGravity_One:
                    ASSERT(s->firstChild == s->lastChild);
                    break;
                case SlGravity_None:
                    DASSERT(0, "A surface with SlGravity_None has children");
                    break;
                default:
                    ASSERT(0, "Write more code here");
            }
        } else {
            // It's a parent with not showing children, or a leaf node.
            //s->allocation.width = max;
        }
    }
}

static inline void
GrowAllocatedWidth(struct SlSurface *s, uint32_t min) {

    DASSERT(s);

}

static inline void
ShrinkAllocatedHeight(struct SlSurface *s, uint32_t max) {

    DASSERT(s);

}

static inline void
GrowAllocatedHeight(struct SlSurface *s, uint32_t min) {

    DASSERT(s);

}


static
void GetChildrenWidgetPositions(struct SlSurface *s) {

    DASSERT(s);
    DASSERT(s->showing);
    DASSERT(s->showingChildren);
    DASSERT(s->firstChild);
    DASSERT(s->lastChild);

    // Start at parent x,y plus a borderWidth.
    uint32_t borderWidth = s->borderWidth;
    uint32_t x = s->allocation.x + borderWidth;
    uint32_t y = s->allocation.y + borderWidth;

    switch(s->gravity) {
 
        case SlGravity_TB:// top to bottom
            // Vertically stack children in order.
           for(struct SlSurface *sf = s->firstChild; sf;
                    sf = sf->nextSibling) {
                sf->allocation.x = x;
                sf->allocation.y = y;
                y += sf->allocation.height + borderWidth;
                if(sf->showingChildren)
                    GetChildrenWidgetPositions(sf);
            }
            break;
        case SlGravity_BT:// bottom to top
            // Vertically stack children in reverse order.
            for(struct SlSurface *sf = s->lastChild; sf;
                    sf = sf->prevSibling) {
                sf->allocation.x = x;
                sf->allocation.y = y;
                y += sf->allocation.height + borderWidth;
                if(sf->showingChildren)
                    GetChildrenWidgetPositions(sf);
            }
            break;
        case SlGravity_LR:// left to right
            // Lineup children in order.
            for(struct SlSurface *sf = s->firstChild; sf;
                    sf = sf->nextSibling) {
                sf->allocation.x = x;
                sf->allocation.y = y;
                x += sf->allocation.width + borderWidth;
                if(sf->showingChildren)
                    GetChildrenWidgetPositions(sf);
            }
            break;
        case SlGravity_RL:// right to left
            // Lineup children in reverse order.
            for(struct SlSurface *sf = s->lastChild; sf;
                    sf = sf->prevSibling) {
                sf->allocation.x = x;
                sf->allocation.y = y;
                x += sf->allocation.width + borderWidth;
                if(sf->showingChildren)
                    GetChildrenWidgetPositions(sf);
            }
            break;
        case SlGravity_One:
            ASSERT(s->firstChild == s->lastChild);
            struct SlSurface *sf = s->lastChild;
            sf->allocation.x = x;
            sf->allocation.y = y;
            break;
        case SlGravity_None:
            DASSERT(0, "A surface with SlGravity_None has children");
            break;
        default:
            ASSERT(0, "Write more code here");
    }


}


static inline
void GetStrideAndStuff(struct SlWindow *win) {

    win->stride = win->surface.allocation.width * SLATE_PIXEL_SIZE;
    size_t sharedBufferSize = win->stride * win->surface.allocation.height;
    if(win->sharedBufferSize != sharedBufferSize && win->buffer)
        // The shared memory pixel buffer will need to be rebuilt.
        FreeBuffer(win);
    win->sharedBufferSize = sharedBufferSize;
    win->needAllocate = false;
}


void slWindow_compose(struct SlWindow *win) {

    DASSERT(win);
    ASSERT(win->surface.parent == 0, "Not a top level window");

    if(!win->needAllocate) {
        WARN("This window is composed already");
        return;
    }

    if(!win->surface.firstChild) {
        // win is a window with no widgets in it.  That's one of the main
        // points of libslate.so; is having a window that the user can
        // draw whatever they want on it, without the widget bullshit
        // making them write a fuck load of code just to get a desktop
        // drawing window.
        //
        // Keeping the simple.  In a sense, making hello-wayland easier
        // to code.
        win->surface.allocation.width  = win->width;
        win->surface.allocation.height = win->height;
        GetStrideAndStuff(win);
        return;
    }

    // Save theses old values.  We need to see if they change.
    uint32_t oldWidth = win->surface.allocation.width,
            oldHeight = win->surface.allocation.height;
    bool oldShowing = win->surface.showing;

    if(!oldShowing)
        // We must pretend that the window is showing so we get some
        // width, and height, for the window.
        win->surface.showing = true;

    DASSERT(win->width  >= win->wWidth);
    DASSERT(win->height >= win->wHeight);

    win->surface.width  = win->width  - win->wWidth;
    win->surface.height = win->height - win->wHeight;

    AddSizeOfSurface(&win->surface, true);

    DASSERT(win->surface.allocation.width);
    DASSERT(win->surface.allocation.height);

    if(oldWidth) {
        DSPEW("old allocation width,height=%" PRIu32 ",%" PRIu32
                "  tallied allocation width,height=%" PRIu32 ",%" PRIu32,
                oldWidth, oldHeight,
                win->surface.allocation.width,
                win->surface.allocation.height);
    } else {
        DSPEW("tallied  allocation width,height = %" PRIu32 ",%" PRIu32,
                win->surface.allocation.width,
                win->surface.allocation.height);
    }

    // Now we have all the size allocations figured out as if we could
    // give all the widgets the sizes they request; but we cannot
    // necessarily give all the widgets the sizes they request.

    if(win->surface.allocation.width > win->surface.width)
        ShrinkAllocatedWidth(&win->surface, win->surface.width);
    else if(win->surface.allocation.width < win->surface.width)
        GrowAllocatedWidth(&win->surface, win->surface.width);

    if(win->surface.allocation.height > win->surface.height)
        ShrinkAllocatedHeight(&win->surface, win->surface.height);
    else if(win->surface.allocation.height < win->surface.height)
        GrowAllocatedHeight(&win->surface, win->surface.height);

    // We define this for the top parent window:
    DASSERT(!win->surface.allocation.x);
    DASSERT(!win->surface.allocation.y);

    // Get the child widgets x and y positions.
    if(win->surface.showingChildren)
        GetChildrenWidgetPositions(&win->surface);

    // We know what we want for the width and height of the window and its
    // widgets.   We need to put back the showing state of the window if
    // it's not showing.
    if(!oldShowing)
        win->surface.showing = oldShowing;

    GetStrideAndStuff(win);
}


void slWindow_show(struct SlWindow *win, bool dispatch) {

    DASSERT(win);
    DASSERT(win->xdg_surface);

    win->surface.showing = true;

    if(ShowSurface(win, dispatch)) {
        DASSERT(0, "ShowSurface() failed");
    }
}
