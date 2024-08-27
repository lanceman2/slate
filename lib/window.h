// Slate window types:
//
//   toplevel
//   popup
//   sub (has a wl_subsurface)
//   fullscreen

enum SlWindowType {

    SlWindowType_topLevel, // From xdg_surface_get_toplevel()
    SlWindowType_popup, // From wl_shell_surface_set_popup()
    SlWindowType_sub, // From wl_subcompositor_get_subsurface()
    SlWindowType_fullscreen
};


struct SlWindow {

    enum SlWindowType type;

    // The parent owns this object.  TopLevel windows have
    // parent=0.
    struct SlWindow *parent;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;

    // TODO: There are other kinds of windows.
    struct xdg_toplevel *xdg_toplevel;

    struct wl_buffer *buffer;

    struct wl_callback *wl_callback;

    int (*draw)(struct SlWindow *win, void *pixels,
            uint32_t w, uint32_t h, uint32_t stride);

    // This is where the shared memory pixels start:
    void *shm_data;
    
    bool configured, open;

    int width, height; // in pixels
};


struct SlToplevel {

    // inherit SlWindow
    struct SlWindow window;

    struct SlToplevel *prev, *next;

    // The display that owns this object.
    struct SlDisplay *display;
};


// slDisplay_destroy() needs to call this.
extern void _slWindow_destroy(struct SlDisplay *d, struct SlWindow *w);

