
// A widget get a portion of a window surface to draw on.

struct SlWidget {

    // inherit slate surface
    struct SlSurface surface;

    // 2D space greediness.  0 mean that this widget does not feel the
    // need to expand into the space that is available.
    enum SlGreed greed;
};


extern void DestroyWidget(struct SlWidget *widget);

