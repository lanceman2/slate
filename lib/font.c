#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "font.h"
#include "debug.h"
#include "window.h"

#include "../include/slate.h"


static uint32_t isInitCount = 0;
static FT_Library library;

static
bool _SlFont_init(void) {

    if(isInitCount++) return false;

    FT_Error error = FT_Init_FreeType(&library);
    RET_ERROR(!error, true, "FT_Init_FreeType() failed error=%d", error);

    return false; // false == success
}

static
bool _SlFont_cleanup(void) {

    DASSERT(isInitCount);
    if(--isInitCount) return false;

    FT_Error error = FT_Done_FreeType(library);
    ASSERT(!error, "FT_Done_FreeType() failed");
    return false; // success
}


static inline
void DrawBgColor(struct SlWindow *win, uint32_t bgColor,
        int32_t x, int32_t y0, uint32_t w, uint32_t h) {

    if(x < 0) {
        if(w < -x)
            return;
        w += x;
        x = 0;
    }
    if(y0 < 0) {
        if(h < -y0)
            return;
        h += y0;
        y0 = 0;
    }

    int32_t xend = x + w;
    if(xend > win->width)
        xend = win->width;
    if(x >= xend)
        return;

    int32_t yend = y0 + h;
    if(yend > win->height)
        yend = win->height;
    if(y0 >= yend)
        return;

    uint32_t width = win->width;
    uint32_t *pixels = win->pixels;

    while(x < xend) {
        int32_t y = y0;
        while(y < yend) {
            pixels[x + y * width] = bgColor;
            ++y;
        }
        ++x;
    }
}


// Draw text using libfreetype2.so.
//
bool slWindow_DrawText(struct SlWindow *win,
        const char *text, const char *font,
        // x,y,w,h in pixels
        int32_t x, int32_t y, uint32_t w, uint32_t h,
        double angle/*in radians*/,
        uint32_t bgColor,
        uint32_t fgColor) {

    if(!text || !*text) {
        // TODO: What to do in this case?
        WARN("No text set");
        return false;
    }
    DASSERT(w > 0);
    DASSERT(h > 0);
    DASSERT(win);
    DASSERT(win->type == SlWindowType_topLevel);
    DASSERT(win->width > 0);
    DASSERT(win->height > 0);

    char *ffile = slFindFont(font);
    if(!ffile) return true; // fail

    bool ret = true; // default return value is true == failure

    struct SlFont f;
    if(_SlFont_init())
        goto init;

    FT_Error error = FT_New_Face(library, ffile, 0, &f.face);
    if(error) {
        ERROR("FT_New_Face(,\"%s\",) failed", ffile);
        goto face;
    }
    DSPEW("Loaded font file %s", ffile);

    /* 50pt at 100dpi.  The 64 seems to be a part of libfreetype2.so. */
    //error = FT_Set_Char_Size(f.face, h * 64, 0, 0, 0 );
    error = FT_Set_Pixel_Sizes(f.face, 0, h);
    if(error) {
        ERROR("FT_Set_Char_Size() failed");
        goto set;
    }
    DSPEW("face height=%ld", f.face->size->metrics.height/64);

    FT_GlyphSlot slot = f.face->glyph;

    // What is this number (0x10000L) for?
    f.matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    f.matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    f.matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    f.matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 Cartesian space coordinates; */
    f.pen.x = 0;
    f.pen.y = 0;

    uint32_t *pixels = win->pixels;
    uint32_t wwidth = win->width;
    uint32_t wheight = win->height;

    const char *end = text + strlen(text);

    // Draw the background color
    DrawBgColor(win, bgColor, x, y, w, h);

    while(text < end) {
        FT_Set_Transform(f.face, &f.matrix, &f.pen);

        error = FT_Load_Char(f.face, *text, FT_LOAD_RENDER);
        if(error) {
            // TODO: What should we do in this bad error case?
            ERROR("FT_Load_Char(,%c,) failed", *text);
            goto set;
        }

//
//
//             *************************************
//             * window shared memory pixels (X,Y) *    
//             *                                   *    
//             *                                   *    
//             *         ----------------          *
//             *         |              |          *        
//             *         |  single      |          *        
//             *         |  char        |          *        
//             *         |  bitmap      |          *
//             *         |              |          *
//             *         ----------------          *
//             *                                   *
//             *                                   *
//             *************************************
//
//
// Note: the "single char bitmap" could be positioned anywhere relative to
// the "window shared memory pixels".
//
// The positions in bitmap space bitmap->buffer[] are of the same
// index/pixel size as the win->pixels[] but they are shifted.  They may
// of may not even overlay.  The parts that do not overlay we do not draw
// to win->pixels[].
//
        const FT_Bitmap *bitmap = &slot->bitmap;

        int32_t b_width = bitmap->width;
        int32_t b_rows = bitmap->rows;
 
        if(b_rows <= 0 || b_width <= 0)
            // The char has no glyph image. It's a space or like thing.
            goto move_pen;

        // Draw to pixels into shared memory at starting at (X,Y).
        int32_t X = x + slot->bitmap_left;
        int32_t Y = y - slot->bitmap_top + h;
        //int32_t Y = y - slot->bitmap_top + f.face->size->metrics.height/64;
        //int32_t Y = y + h - f.face->size->metrics.height/64;
        //int32_t Y = y - slot->bitmap_top + h +
          //          (h - f.face->size->metrics.height/64);
        int32_t Y0 = Y;

        // The image of glyph in pixel space is at X,Y and
        // (width,height) bitmap->width, bitmap->rows
        int32_t xpix_end = X + bitmap->width;
        int32_t ypix_end = Y0 + bitmap->rows;
        int32_t bitmapWidth = bitmap->width;

        if(xpix_end > wwidth)
            // constrain by the window
            xpix_end = wwidth;
        if(xpix_end > x + w)
            // constrain by the function x,w parameters
            xpix_end = x + w;

        if(ypix_end > wheight)
            // constrain by the window
            ypix_end = wheight;
        if(ypix_end > y + h)
            // constrain by the function y,h parameters
            ypix_end = y + h;

        uint32_t i0 = 0;
        uint32_t j0 = 0;

        if(X < 0) {
            // The left edge of the bitmap is to the left
            // of the window shared memory pixel buffer.
            i0 = -X;
            X = 0;
        }
        if(Y0 < 0) {
            // The top edge of the bitmap is above the window shared
            // memory pixel buffer.
            j0 = -Y;
            Y = Y0 = 0;
        }

#if 1
        fprintf(stderr, "[%c] x,y=(%" PRIu32 ",%" PRIu32 ") end=("
                "%" PRIi32 ",%" PRIi32 ") "
                "i0,j0=(%" PRIu32 ",%" PRIu32 ")\n", *text,
                X, Y, xpix_end, ypix_end, i0, j0);
#endif

        // We only iterate over values where the X,Y and bitmap spaces
        // overlap.  If they do not overlap than we get no values.
        //
        // TODO: quit the outer loop if we have no more overlap.
        //
        int32_t i = i0;

        // Note X is X0 is X.
        while(X < xpix_end && i < b_width) {
            int32_t j = j0;
            Y = Y0;
            while(Y < ypix_end && j < b_rows) {
                unsigned char val = bitmap->buffer[i + j*bitmapWidth];
                pixels[X + Y*wwidth] = 0xF0000000 +
                    (val << 16) + (val << 8) + val;
                ++Y;
                ++j;
            }
            ++i;
            ++X;
        }

move_pen:

        /* increment pen position */
        f.pen.x += slot->advance.x;
        f.pen.y += slot->advance.y;

        text++;
    }

    ret = false; // success.
    PushPixels(win);

set:
    FT_Done_Face(f.face);
face:
    _SlFont_cleanup();
init:
    free(ffile);
    return ret; // false == success
}

