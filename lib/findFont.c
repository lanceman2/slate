// This started from code at:
// https://stackoverflow.com/questions/10542832/how-to-use-fontconfig-to-get-font-list-c-c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

#include "debug.h"
#include "../include/slate.h"


#define FAIL(to, fmt, ...) \
    do {\
        ERROR(fmt, ##__VA_ARGS__);\
        goto to;\
    } while(0)


// This returns a file name as a string pointer that must be free(3)ed.
//
// The question here is should we be keeping around some of the
// intermediate libfontconfig state data?  If this is just called once in
// a program process than the answer is most likely, No.  Maybe it will be
// called 3 times, and in that case the answer could still be, no.
//
// TODO: libfontconfig.so uses signed char for strings, WTF (what the
// fuck), so should I be checking an setting the all the sign bits to
// zero?  I always wondered why the sign bit was ignored in strings.
//
unsigned char *SlFindFont(const unsigned char *exp) {

    RET_ERROR(exp, 0, "SlFindFont(exp=0) failed");

    FcConfig *config;
    FcPattern *pat;
    FcPattern *font;
    unsigned char *file = 0;

    RET_ERROR(FcInit(), 0, "FcInit() failed");

    config = FcInitLoadConfigAndFonts();
    if(!config)
        FAIL(config, "FcInitLoadConfigAndFonts() failed");

    pat = FcNameParse(exp);
    if(!pat)
        FAIL(pat, "FcNameParse(\"%s\") failed", exp);

    FcBool ret = FcConfigSubstitute(config, pat, FcMatchPattern);
    if(!ret)
        FAIL(pat, "FcConfigSubstitute() failed");

    FcDefaultSubstitute(pat);

    FcResult result;
    font = FcFontMatch(config, pat, &result);
    if(!font)
        FAIL(font, "FcFontMatch() failed");

    if(FcPatternGetString(font, FC_FILE, 0, &file) != FcResultMatch) {
        DASSERT(file == 0);
        FAIL(all, "FcPatternGetString() failed");
    }

    DASSERT(file);
    // TODO: WTF is with unsigned
    file = (unsigned char *) strdup((const char *) file);
    ASSERT(file, "strdup() failed");

    // Cleanup in reverse order on your way out of this function:
all:
    FcPatternDestroy(font);
font:
    FcPatternDestroy(pat);
pat:
    FcConfigDestroy(config);
config:
    FcFini();

    return file;
}
