// This started from code at:
// https://stackoverflow.com/questions/10542832/how-to-use-fontconfig-to-get-font-list-c-c


// We added code that was needed so this passed the valgrind test and so
// it did not spew so much.

#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

#include "../lib/debug.h"


int main(void) {

    // ref: https://gist.github.com/CallumDev

    RET_ERROR(FcInit(), 1, "FcInit() failed");

    FcConfig *config = FcInitLoadConfigAndFonts(); // Most convenient of all the alternatives.
    RET_ERROR(config, 1, "FcInitLoadConfigAndFonts() failed");

    // The string does not necessarily have to be a specific name.  You
    // could put anything here and Fontconfig WILL find a font for you.
    FcPattern* pat = FcNameParse("Arial");
    RET_ERROR(pat, 1, "FcNameParse() failed");

    //NECESSARY; it increases the scope of possible fonts
    FcBool ret = FcConfigSubstitute(config, pat, FcMatchPattern);
    RET_ERROR(ret, 1, "FcConfigSubstitute() failed");
    FcDefaultSubstitute(pat); //NECESSARY; it increases the scope of possible fonts

    FcResult result;

    FcPattern* font = FcFontMatch(config, pat, &result);

    if(font) {
        // The pointer stored in 'file' is tied to 'font'; therefore, when
        // 'font' is freed, this pointer is freed automatically.  If you
        // want to return the filename of the selected font, pass a buffer
        // and copy the file name into that buffer.
	FcChar8* file = 0; 
	if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
	    fprintf(stderr, "%s\n", file);
    }

    FcPatternDestroy(font);// needs to be called for every pattern created;
                           // in this case, 'fontFile' / 'file' is also
                           // freed.
    FcPatternDestroy(pat); // needs to be called for every pattern created
    FcConfigDestroy(config); // needs to be called for every config created
    FcFini(); // uninitializes Fontconfig
    return 0;
}
