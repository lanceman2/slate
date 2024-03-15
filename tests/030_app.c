
#include "../include/slate.h"


int main(void) {

    struct SlApp *app = slApp_create();

    slApp_destroy(app);

    return 0;
}
