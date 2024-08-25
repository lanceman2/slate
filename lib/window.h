

struct SlWindow {

    struct SlDisplay *display;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;

    // TODO: There are other kinds of windows.
    struct xdg_toplevel *xdg_toplevel;

    struct wl_buffer *buffer;

    struct wl_callback *wl_callback;

    int (*draw)(struct SlWindow *, void *pixels, size_t size);

    // This is where the shared memory pixels start:
    void *shm_data;
    
    bool configured, open;

    struct SlWindow *prev, *next;

    int width, height; // in pixels
};


// slDisplay_destroy() needs to call this.
extern void _slWindow_destroy(struct SlDisplay *d, struct SlWindow *w);

