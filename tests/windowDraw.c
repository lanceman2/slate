#include <stdbool.h>
#include <signal.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"


void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}



int main(void) {

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    struct SlDisplay *d = slDisplay_create();

    const uint32_t NUM_WINS = 1;

    struct SlWindow *w[NUM_WINS];

    for(int i=0; i<NUM_WINS; ++i)
        w[i] = slWindow_createTop(d, 100, 100, i*100, 10);

    ERROR("w[0]=%p", w[0]);

    while(slDisplay_dispatch(d)) {


    }

    slDisplay_destroy(d);


    DSPEW("DONE1");


    d = slDisplay_create();

    for(int i=0; i<NUM_WINS; ++i)
        w[i] = slWindow_createTop(d, 100, 100, i*100, 10);

    while(slDisplay_dispatch(d)) {


    }

    ASSERT(0);

    slDisplay_destroy(d);

    DSPEW("DONE2");



    return 0;
}
