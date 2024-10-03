// Slate window types:
//
//   toplevel
//   popup
//   sub (has a wl_subsurface)
//   fullscreen???

enum SlSurfaceType {

    // First are window types:
    SlSurfaceType_topLevel = 0xFB57, // From xdg_surface_get_toplevel()
    SlSurfaceType_popup, // From wl_shell_surface_set_popup()

    //SlSurfaceType_sub, // From wl_subcompositor_get_subsurface()
    //SlSurfaceType_fullscreen, // ??? fullscreen

    // SlSurfaceType_widget MUST BE LAST AND LARGEST.
    SlSurfaceType_widget // The only surface that is not a window
};


// Allocation is all the parameters that need to change when a window (and
// the widgets there-in) is resized, or first configured (except stride).
// Similar to what GTK calls an allocation (theirs is just the rectangle),
// but we include the starting pixel pointer.  The stride that is needed
// to draw is the same for all the widgets in the window, so it's not in
// this SlAllocation thingy.
//
// SlAllocation is part of SlSurface.
//
struct SlAllocation {

    // A pixel allocation parameterizes the part of the pixels a widget
    // (or window) can draw on.  allocation::x,y is the position relative
    // to the toplevel parent window surface.  The word allocation is like
    // GTK uses the word for, except we use positions relative to the
    // toplevel window.  We can get the relative X position from:
    //
    //   x_rel = widget->allocation.x - widget->parent->allocation.x
    //   y_rel = widget->allocation.y - widget->parent->allocation.y
    //
    // Note it's not like GTK: GTK allocation uses positions x,y relative
    // to the parent, not relative to the GDK window position.
    //
    // Calculating positions relative to the toplevel window, for relative
    // positions, is a pain-in-the-ass in GTK.  We'll see if this works
    // out...
    //
    uint32_t x, y, width, height;

    // "pixels" points to where the inter-process shared memory pixels
    // start for the case of a window, and "pixels" points to the top left
    // corner of the rectangle for a widget that is inside of a window.
    uint32_t *pixels;
};


// A Window or a Widget are a Surface (or have a surface in them).
//
struct SlSurface {

    enum SlSurfaceType type;

    // The toplevel window allocation::x,y will always be 0,0.
    //
    // Current allocation.  This can change as the surface is resized from
    // the wayland desktop window manager/compositor server.
    //
    struct SlAllocation allocation;


    // We keep a linked list (tree like) graph of surfaces starting at a
    // window with parent == 0.  The top level parent windows are owned by
    // a display.  Displays are owned by a static global list of displays
    // in display.c.
    //
    struct SlSurface *parent;
    struct SlSurface *firstChild, *lastChild;
    struct SlSurface *nextSibling, *prevSibling;

    // Like the direction of a gravitational field the in this surface
    // area.  This "gravity" effects how would be child widgets are packed
    // inside this parent surface.  "gravity" is 0 for a surface that
    // cannot have children.
    enum SlGravity gravity;

    // width,height preferred/requested by the user API create function:
    // slWindow_create() or slWidget_create().
    uint32_t width, height; // in pixels


    uint32_t backgroundColor;
    // This surface will add this width of border between all its
    // children.  This is ignored if there are no children.
    uint32_t borderWidth;

    int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride);

    // State saved after the first tree traversal so we have it for the
    // next traversal, in slWindow_compose() function which computes
    // widget width and height allocations.
    //
    // The SlSurface::showing flag is set and kept based on the API user
    // calls, but "showingChildren" is set based on looking at the widget
    // tree as we traverse it.  "showingChildren" saves having to do more
    // widget tree traversals, at the cost of (one bool) memory.
    //
    bool showingChildren;

    // This, "showing", is a API user requested attribute of the surface.
    // It has two meanings that depend on if it is a top most surface
    // (window) or a widget.
    //
    // TOP MOST: For the top most surface parent this hide (showing == 0)
    // means do not render the window. Iconify is a different thing.  The
    // window manager controls iconify and a showing window can be
    // iconified.  This "showing/hidden" state has nothing to do with the
    // window manager's iconify state.
    //
    // WIDGET: For a widget (not a top most surface parent) showing == 0
    // means do not allocate space for this widget's surface.  If showing
    // == 0 then do not allocate space for this widget's surface or it's
    // children either.  If a widget has at parent with showing == 0 then
    // the widget will not get allocated space or be shown in the display.
    //
    // We chose the word "showing" because we wanted its default value, 0,
    // to correspond to not showing.  The widget/window can be showing
    // after all its children added to it.
    //
    // showing does not tell you if the current widget is displayed.  It only
    // marks that the widget will have space allocated for it when the
    // window and its' child widgets are composed and displayed.
    //
    // The gist of it is:
    //
    //   hidden (showing == 0) widgets do not get allocated space.
    //
    // This is not so much like GTK's "show" and "hide".  Maybe it's
    // more like GTK's "visible".
    //
    bool showing; // showing == true --> make space for it
};


struct SlWindow {

    // inherit slate surface.  We keep this first in the structure so
    // that we may call slWidget_create((void *) window, ...)
    struct SlSurface surface;

    // "stride" is the distance in bytes from positions X,Y to get to the
    // next (X, Y+1) position at a same X value.  It's used to loop back
    // to the next Y row in the pixels data.
    //
    // Think of X position as increasing as you move along (increasing X)
    // a row.
    //
    // "stride" is 4*width for a window because each pixel is 4 bytes in
    // size and there is no memory padding at end of a row.  It just works
    // out that way.  For a widget that is not as wide as the window that
    // contains it the stride is larger than 4*width where width is the
    // width of the widget.
    //
    // what-does-stride-mean: How long with this URL last?:
    // https://medium.com/@oleg.shipitko/what-does-stride-mean-in-image-processing-bba158a72bcd
    //
    // The child widgets have this same stride.
    //
    uint32_t stride;

    // Size in bytes of the current mmap() shared memory used with the
    // wayland compositor and this wayland client.  This is a function of
    // the width and height in the surface.allocation (above), but it
    // may not be in the case where we are in the midst of resizing the
    // window.  Hence we need the old shared memory size, in order to call
    // munmap(2) to cleanup.
    size_t sharedBufferSize;

    // The parent owns this object.  TopLevel windows have
    // parent=0.
    struct SlWindow *parent;

    // We store/get a pointer to this SlWindow in the wl_surface user_data
    // using: wl_surface_set_user_data() and wl_surface_get_user_data().
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;

    struct wl_buffer *buffer;

    struct wl_callback *wl_callback;

    // For the doubly linked list of children in the toplevel
    // (firstChild, lastChild) windows.
    struct SlWindow *prev, *next;


    // Negative values put the window at a root window edge.  Examples:
    //   -1,0  ==> right,top
    //    0,-1 ==> left,bottom
    //   -1,-1 ==> right,bottom
    //
    int32_t x, y;

    bool configured, open, framed;

    // When the user is creating and assembling widgets into a window the width and
    // height of widgets need to be calculated via slWindow_compose().
    bool needAllocate;

    // When the wayland pixel buffer needs recreating.  If needAllocate is
    // true then this needs to be true too.  So we have just 3 valid
    // states for the two flags needAllocate and needReconfigure.
    //
    bool needReconfigure; // TODO: merge this with configure flag?
};


struct SlToplevel {

    // inherit SlWindow
    struct SlWindow window;

    struct xdg_toplevel *xdg_toplevel;

    struct SlToplevel *prev, *next;

    // TODO: Does wayland client expose the parent's children?
    // Or do we really need to keep this list of children?
    struct SlWindow *firstChild, *lastChild;

    // The display that owns this toplevel object.
    struct SlDisplay *display;
};


struct SlPopup {

    // inherit SlWindow
    struct SlWindow window;

    struct SlToplevel *parent;

    struct xdg_popup *xdg_popup;
    struct xdg_positioner *xdg_positioner;
};



extern void _slWindow_destroy(struct SlDisplay *d, struct SlWindow *w);


extern bool CreateWindow(struct SlDisplay *d, struct SlWindow *win,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride));

// Toplevel windows can have children.  These Add and Remove from the
// toplevel's list of child windows.
extern void AddChild(struct SlToplevel *t, struct SlWindow *win);
//
extern void RemoveChild(struct SlToplevel *t, struct SlWindow *win);

extern bool ConfigureSurface(struct SlWindow *win, bool dispatch);

// TODO: Damage just a rectangular region of interest.
extern void PushPixels(struct SlWindow *win);



