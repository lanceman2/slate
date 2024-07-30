

struct SlWindow {

    struct SlDisplay *display;

    struct wl_surface *wl_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct xdg_surface *xdg_surface;


    struct SlWindow *prev, *next;

    int width, height; // in pixels
};


// slDisplay_destroy() needs to call this.
extern void _slWindow_destroy(struct SlDisplay *d, struct SlWindow *w);
