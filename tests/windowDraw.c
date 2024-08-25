#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}

static int color = 100;

static int draw_count = 0;

static
int draw(struct SlWindow *win, void *pixels, size_t size) {

draw_count++;
DSPEW("                                      draw_count=%d", draw_count);


    memset(pixels, color, size);

    color++;

    //return 1; // stop calling

    return 0; //continue to calling at every frame, like at 60 Hz.
}

int main(void) {

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();

    slWindow_createTop(d, 100, 100, 100, 10, draw);

    fprintf(stderr, "\n\nHIT <Alt-F4> to exit\n\n");

    while(slDisplay_dispatch(d));


    // Use automatic cleanup from the libslate.so destructor.

    // If we make it here it does not seen to crash.
    DSPEW("DONE");

    return 0;
}
