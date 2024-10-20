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
    //   x_rel = surface->allocation.x - surface->parent->allocation.x
    //   y_rel = surface->allocation.y - surface->parent->allocation.y
    //
    //
    // Calculating positions relative to the toplevel window.  We'll see
    // if this works out...
    //
    // Note it's not like GTK: GTK allocation uses positions x,y relative
    // to the parent, not relative to the GDK window position.

    //
    uint32_t x, y, width, height;
};


// A Window or a Widget are a Surface (or have a surface in them).
//
struct SlSurface {

    enum SlSurfaceType type;

    // The window that owns this surface.  This is needed for widgets that
    // are drawn on windows.  If window is 0 than this, SlSurface, is part
    // of a SlWindow that is the window.
    //struct SlWindow *window;

    // The toplevel window allocation::x,y will always be 0,0.
    //
    // Current allocation.  This can change as the surface is resized from
    // the wayland desktop window manager/compositor server.
    //
    struct SlAllocation allocation;

    // We keep a linked list (tree like) graph of surfaces starting at a
    // window with parent == 0.  The top level parent windows are owned by
    // a SlDisplay (display).  Displays are owned by a static global list
    // of displays in display.c.
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

    // We keep a queue as a linked list of frame draw surfaces with
    // SlWindow::DrawQueue dq1,dq2
    struct SlSurface *next;

    int (*draw)(struct SlSurface *surface, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride);

    void (*getChildrenPosition)(struct SlSurface *surface,
            uint32_t width, uint32_t height,
            uint32_t childrenWidth, uint32_t childrenHeight,
            uint32_t *childrenX, uint32_t *childrenY);

    // State saved after the first tree traversal of the many needed for
    // getting the surface allocations so we have it for the next
    // traversal, in slWindow_compose() function which computes widget
    // width and height (and x, y) allocations.
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

    bool queued; // Is this surface queued in the drawFrame?
};


// A queue is a linked list of surfaces.
//
struct DrawQueue {
    struct SlSurface *last, *first;
};


struct SlWindow {

    // inherit slate surface.  We keep this first in the structure so
    // that we may call slWidget_create((void *) window, ...)
    struct SlSurface surface;

    // We needed two queues because one is emptied while the other is
    // being loaded; and we switch between them before reading and
    // emptying a queue.
    struct DrawQueue dq1, dq2, *writingDQ, *readingDQ;

    // "stride" is the distance in bytes from positions X,Y to get to the
    // next (X, Y+1) position at a same X value.  It's used to loop back
    // to the next Y row in the pixels data.
    //
    // Think of X position as increasing as you move along (increasing X)
    // a row.
    //
    // "stride" is 4*width for a window because each pixel is 4 bytes in
    // size and there is no memory padding at the end of a row.  It just
    // works out that way.  For a widget that is not as wide as the window
    // that contains it, the stride is larger than 4*width where width is
    // the width of the widget.
    //
    // what-does-stride-mean: How long will this page (URL) last?:
    // https://medium.com/@oleg.shipitko/what-does-stride-mean-in-image-processing-bba158a72bcd
    //
    // The child widgets have this same stride.
    //
    uint32_t stride;

    // "pixels" is a pointer to the start of the mmap(2) shared memory
    // that is shared between this libwayland-client.so process and the
    // compositor (server).
    uint32_t *pixels;

    // If this is a widget container all 6 of the next parameters are
    // used: the window itself has extra space from width, height (for
    // itself) and the children are in a rectangle given by wX, wY,
    // wWidth, wHeight.

    // Preferred total width, height by the API user.
    uint32_t width, height;

    // The child widgets (w) [if any] preferred position and size.
    uint32_t wX, wY, wWidth, wHeight;


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

    // Wayland tends to have a lot of fine grain user interface structure
    // as we see from the 5 wayland structures below plus the 8 process
    // level display like structures.  The libslate.so API is much less
    // flexible, but has far fewer user interfaces.
    //
    // The idea of the libslate.so API is to draw to pixels on a window
    // with just 5 lines of C code.  Orders of magnitude less lines of
    // code.  And (in the other direction) an order of magnitude less
    // dependency, portability code, and user code, than when using GTK
    // and Qt (bloat monsters); with much better performance; maybe a
    // whole (1 layer of) pixel buffer memory copy less.  Or is that
    // so...
    //
    // We give the libslate.so user the ability to avoid having more
    // layers pixel memory copying.  Yes, we know that there is an
    // unavoidable memory copy from the shared pixel memory to whatever
    // the compositor server writes to to show pixels to the end user, but
    // we let the libslate.so user limit the memory copying in their code.
    // GTK and Qt make it hard, if not impossible, to have user code that
    // directly writes to the wayland client/server shared memory pixels.
    // That means we should be measurable increases in 2D drawing speed.
    //
    // I'd expect that the OpenGL windows in GTK and Qt have removed
    // unnecessary pixel memory copying that I speak of.  Q: Can we use
    // the OpenGL GTK (or Qt) window thingy as a high performance 2D
    // drawing surface?
    //
    // Q: Is there an interface (in GTK or Qt) to get a pointer to the
    // wayland client/server pixel shared memory?  I have seen exposed
    // wayland client stuff in the Qt interfaces, but not a single example
    // that uses it.  Is it a drawing layer that is accessible to the Qt
    // API user?  With no example it's hard to tell if it's exposed for
    // just internal use (due to an old short-coming of C++) or API user
    // use.  The Qt documentation losses me.  Too many layers.  I can't
    // get from a QWindow to any wayland-client objects that are under it,
    // but I know it has to be there in say a QSurface::RasterSurface.  I
    // can only guess that Qt is designed without exposing any of the
    // libwayland-client.so structures through a C++ user interface.  That
    // makes sense that Qt user API interfaces are not Linux desktop
    // friendly, and hence there is no non-hackish way to access the
    // "wayland shared memory pixels".
    //
    //
    // TODO: We need to see if there is easy access to parameters in this
    // wayland shit, such that we can not bother having redundant
    // manifestations of said parameters in this (SlWindow) structure;
    // like for example surface "width".
    //
    // We store/get a pointer to this SlWindow in the wl_surface user_data
    // using: wl_surface_set_user_data() and wl_surface_get_user_data().
    //
    // Counting the five (5) wayland structures that are specific to a
    // particular slate window.
    struct wl_surface  *wl_surface;  // 1
    struct xdg_surface *xdg_surface; // 2
    struct wl_buffer   *wl_buffer;   // 3
    struct wl_callback *wl_callback; // 4
    struct wl_shm_pool *wl_shm_pool; // 5


    // For the doubly linked list of children (widgets) in the toplevel
    // (firstChild, lastChild) windows.
    //
    // popup windows use this.  Maybe there are other types of windows to
    // come.
    //
    struct SlWindow *prev, *next;

    // Negative values put the window at a root window edge.  Examples:
    //   -1,0  ==> right,top
    //    0,-1 ==> left,bottom
    //   -1,-1 ==> right,bottom
    //
    int32_t x, y;

    // TODO: Too many fucking flags here:
    //
    // Looks like wayland has a configure event that has no widow size
    // when the configure callback is called.  The xdg_surface is created
    // but it has no size.  Looks like we can get more than one of these
    // xdg_surface_configure events for a given window.
    //
    uint32_t xdg_configured;

    bool open, framed;

    // First we allocate widget widths and heights in slWindow_compose().
    //
    // When the user is creating and assembling widgets into a window the
    // width and height of widgets need to be calculated via
    // slWindow_compose().  We do not do that calculation every time a
    // widget is added to a window, so we need a flag so that we can
    // wait to do that later before we draw.
    //
    bool needAllocate;
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
            uint32_t w, uint32_t h, uint32_t stride),
        void (*getChildrenPosition)(struct SlWindow *win,
            uint32_t width, uint32_t height,
            uint32_t childrenWidth, uint32_t childrenHeight,
            uint32_t *childrenX, uint32_t *childrenY)
        );

// Toplevel windows can have children.  These Add and Remove from the
// toplevel's list of child windows.
extern void AddChild(struct SlToplevel *t, struct SlWindow *win);
//
extern void RemoveChild(struct SlToplevel *t, struct SlWindow *win);

extern bool ShowSurface(struct SlWindow *win, bool dispatch);

// TODO: Damage just a rectangular region of interest.
extern void PushPixels(struct SlWindow *win);

extern void FreeBuffer(struct SlWindow *win);

// Calculate all the widget (and window) geometries, positions (x,y)
// widths and heights; and that's all.  Just after calling this the widths
// and heights may not be consistent with what is currently being
// displayed.
void slWindow_compose(struct SlWindow *win);

