// This started from code at:
// https://stackoverflow.com/questions/10542832/how-to-use-fontconfig-to-get-font-list-c-c
// then added like code to ../lib/findFont.c

// We added code that was needed so this passed the valgrind test and so
// it did not spew so much.

#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

#include "../lib/debug.h"
#include "../include/slate.h"


int main(void) {

    const char *font = "NotoSans";
    char *file = slFindFont(font);

    if(!file) return 1;

    fprintf(stderr, "slFindFont(\"%s\") = %s\n", font, file);

    free(file);

    return 0;
}
