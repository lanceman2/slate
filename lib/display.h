

struct SlDisplay {

    // We keep a list of displays in display.c.
    struct SlDisplay *prev, *next;

    // List of slate windows owned by this display.
    struct SlWindow *firstWindow, *lastWindow;

    pthread_mutex_t mutex;
};

