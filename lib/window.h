// Slate window types:
//
//   toplevel
//   popup
//   sub (has a wl_subsurface)
//   fullscreen

enum SlWindowType {

    SlWindowType_topLevel = 1, // From xdg_surface_get_toplevel()
    SlWindowType_popup, // From wl_shell_surface_set_popup()
    SlWindowType_sub, // From wl_subcompositor_get_subsurface()
    SlWindowType_fullscreen // ???
};


struct SlWindow {

    enum SlWindowType type;

    // The parent owns this object.  TopLevel windows have
    // parent=0.
    struct SlWindow *parent;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;

    struct wl_buffer *buffer;

    struct wl_callback *wl_callback;

    int (*draw)(struct SlWindow *win, void *pixels,
            uint32_t w, uint32_t h, uint32_t stride);

    // This is where the shared memory pixels start:
    void *shm_data;

    // For the doubly linked list of children in the toplevel
    // (firstChild, lastChild) windows.
    struct SlWindow *prev, *next;

    bool configured, open, framed;

    uint32_t width, height; // in pixels
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
        int (*draw)(struct SlWindow *win, void *pixels,
            uint32_t w, uint32_t h, uint32_t stride));

// Toplevel windows can have children.  These Add and Remove from the
// toplevel's list of child windows.
extern void AddChild(struct SlToplevel *t, struct SlWindow *win);
//
extern void RemoveChild(struct SlToplevel *t, struct SlWindow *win);

extern bool ConfigureSurface(struct SlWindow *win);


