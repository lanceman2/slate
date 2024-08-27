
// A popup window.

struct SlPopup {

    // inherit SlWindow
    struct SlWindow window;

    struct SlWindow *parent;
};


// slWindow_destroy() needs to call this.
extern void _slPopup_destroy(struct SlWindow *w, struct SlPopup *p);

