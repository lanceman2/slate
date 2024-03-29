// There may be just one wayland display, but we have many slate
// displays that own other data/function things like slate windows.  I
// knew I made this slate display abstraction for some reason.  A
// program can have many modules that have a display (or displays)
// that do not necessarily know about each other (other modules).  And
// the displays in each module can have any number of slate windows.
// It's this idea of modular coding, putting together bits and pieces
// code that do not necessarily know about each other.
//
// The hard part may be parsing/multiplexing/passing the events from the
// one "real" wayland display to the many slate displays.

struct SlDisplay {

    // We keep a list of displays in display.c.
    struct SlDisplay *prev, *next;

    // List of slate windows owned by this display.
    struct SlWindow *firstWindow, *lastWindow;

    pthread_mutex_t mutex;
};

