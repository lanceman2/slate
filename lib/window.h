// Slate window types:
//
//   toplevel
//   popup
//   sub (has a wl_subsurface)
//   fullscreen???

enum SlSurfaceType {

    // First window types:
    SlSurfaceType_topLevel = 1, // From xdg_surface_get_toplevel()
    SlSurfaceType_popup, // From wl_shell_surface_set_popup()

    //SlSurfaceType_sub, // From wl_subcompositor_get_subsurface()
    //SlSurfaceType_fullscreen, // ??? fullscreen

    SlSurfaceType_widget // not a window
};


// Allocation is all the parameters that need to change when a window (and
// the widgets there-in) is resized, or first configured.  Similar to what
// GTK calls an allocation (theirs is just the rectangle), but we include
// the starting pixel pointer and stride.
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
    uint32_t stride;
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
    uint32_t borderWidth;


    int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride);
};


struct SlWindow {

    // inherit slate surface
    struct SlSurface surface;

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

    bool configured, open, framed;

    // Negative values put the window at a root window edge.  Examples:
    //   -1,0  ==> right,top
    //    0,-1 ==> left,bottom
    //   -1,-1 ==> right,bottom
    //
    int32_t x, y;
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

extern bool ConfigureSurface(struct SlWindow *win);

// TODO: Damage just a rectangular region of interest.
extern void PushPixels(struct SlWindow *win);



