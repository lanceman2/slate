#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

#include "../include/slate.h"

#include "debug.h"
#include "window.h"
#include "display.h"
#include "popup.h"
#include "shm.h"




void _slPopup_destroy(struct SlWindow *w, struct SlPopup *p) {

    DASSERT(w);
    DASSERT(p);
    DASSERT(w == p->parent);

}

