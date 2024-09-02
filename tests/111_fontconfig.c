// This started from code at:
// https://stackoverflow.com/questions/10542832/how-to-use-fontconfig-to-get-font-list-c-c


// We added code that was needed so this passed the valgrind test and so
// it did not spew so much.

#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

#include "../lib/debug.h"


int main(void) {

    FcBool result = FcInit();
    RET_ERROR(result, 1, "FcInit() failed");

    FcConfig *config = FcConfigGetCurrent();
    RET_ERROR(config, 1, "FcConfigGetCurrent() failed");

    result = FcConfigSetRescanInterval(config, 0);
    RET_ERROR(result, 1, "FcConfigSetRescanInterval() failed");

    // show the fonts
    FcPattern *pat = FcPatternCreate();
    RET_ERROR(pat, 1, "FcPatternCreate() failed");

    FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_LANG, 0);
    RET_ERROR(os, 1, "FcObjectSetBuild() failed");

    FcFontSet *fs = FcFontList(config, pat, os);
    RET_ERROR(fs, 1, "FcFontList() failed");

    printf("Total fonts: %d\n", fs->nfont);
    for(int i=0; fs && i < fs->nfont; i++) {
        FcPattern *font = fs->fonts[i];//FcFontSetFont(fs, i);
        //FcPatternPrint(font);
        FcChar8 *s = FcNameUnparse(font);
        FcChar8 *file;
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
            //printf("Filename: %s", file);
        }
        //printf("Font: %s", s);
        free(s);
    }

    FcFontSetDestroy(fs);
    FcObjectSetDestroy(os);
    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();

    return 0;
}
