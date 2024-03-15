
#include "../include/slate.h"


int main(void) {

    for(int i=0; i<5; ++i) {
        struct SlDisplay *display = slDisplay_create();
        slDisplay_destroy(display);
    }

    const int N = 6;
    struct SlDisplay *display[N];

    for(int i=0; i<N; ++i)
        display[i] = slDisplay_create();
    for(int i=0; i<N; ++i)
        slDisplay_destroy(display[i]);

    return 0;
}
