

struct SlWindow {

    struct SlDisplay *display;

    struct SlWindow *prev, *next;

    int width, height; // in pixels
    struct xdg_surface *xdg_surface;
};


// slDisplay_destroy() needs to call this.
extern void _slWindow_destroy(struct SlDisplay *d,
        struct SlWindow *w);
