

// A widget get a portion of a window surface to draw on.
//
struct SlWidget {

    // inherit slate surface.  We keep this first in the structure so that
    // we may call slWidget_create((void *) widget, ...)
    struct SlSurface surface;

    // The window that this surface is in, or top most surface.
    struct SlWindow *window;

    // 2D space greediness.  0 mean that this widget does not feel the
    // need to expand into the space that is available.
    enum SlGreed greed;

};


extern void DestroyWidget(struct SlSurface *widget);

