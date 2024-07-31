// Credit:
//
// Most of this C file came from https://github.com/emersion/hello-wayland
//
// A hello world Wayland client, 2018 edition.  The shm.c file.

// It's been changed a lot for this use case.

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"


static void randname(char *buf) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    for (int i = 0; i < 6; ++i) {
        buf[i] = 'A'+(r&15)+(r&16)*2;
	r >>= 5;
    }
}

static int anonymous_shm_open(void) {

    // TODO: Add a bigger name differentiator, so we can run more
    // programs that use this library.

    char name[] = "/slate-XXXXXX";
    int retries = 77;

    do {
        randname(name + strlen(name) - 6);
	--retries;
	// shm_open guarantees that O_CLOEXEC is set
        errno = 0;
	int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
	if(fd >= 0) {
	    if(shm_unlink(name)) {
                ERROR("shm_unlink(\"%s\") failed", name);
                close(fd);
                return -1;
            }
	    return fd;
	}
    } while(retries > 0 && errno == EEXIST);

    ERROR("Failed to get shm_open() file open");

    return -1;
}

int create_shm_file(off_t size) {

    int fd = anonymous_shm_open();
    if(fd < 0)
        return fd;

    if(ftruncate(fd, size) < 0) {
        ERROR("ftruncate(,%jd) failed", size);
	close(fd);
	return -1;
    }

    return fd;
}
