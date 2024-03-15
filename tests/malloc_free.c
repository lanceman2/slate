#include <stdlib.h>

#include "../lib/debug.h"

int main(void) {

    void *ptr = malloc(1);
    ASSERT(ptr, "malloc(1) failed");

    free(ptr);

    return 0;
}
