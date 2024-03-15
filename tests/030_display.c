
#include "../include/slate.h"


int main(void) {

    struct SlDisplay *display = slDisplay_create();
    slDisplay_destroy(display);

    return 0;
}
