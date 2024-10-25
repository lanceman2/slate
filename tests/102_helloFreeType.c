/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <inttypes.h>

#include "../lib/debug.h"
#include "../include/slate.h"

#include <ft2build.h>
#include FT_FREETYPE_H


#define WIDTH   640
#define HEIGHT  480

static
void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}

/* origin is the upper left corner */
static unsigned char text_image[HEIGHT][WIDTH] = {0};


static int draw_count = 0;

static
int draw(struct SlWindow *win, uint32_t *pix,
            uint32_t w, uint32_t h, uint32_t stride) {

    // Line stride (increment, pitch or step size) is the number of bytes
    // that one needs to add to the address in the first pixel of a row in
    // order to go to the address of the first pixel of the next row.
    draw_count++;
    fprintf(stderr, "               draw_count=%d    \r", draw_count);

    // Each pixel it 4 bytes or the sizeof(uint32_t) = 4 bytes.
    // linePad is the distance in pixels to the end of a x row
    // from the last pixel drawn.  linePad is likely 0.
    // Note: stride >= w * 4 (width*4).
    const uint32_t linePad = stride/4 - w;
    for(uint32_t y=0; y<h; y++) {
        for(uint32_t x=0; x<w; ++x) {

            // ARGB color pix is for example
            // like 0x0AFF0022 is alpha=0A red=FF green=00 blue=22
            if(x >= 30 && x < WIDTH+30 && y >= 20 && y < HEIGHT+20) {

                uint32_t p = text_image[y-20][x-30];
                *pix = 0xAF000000 | (p << 16) | (p << 8) | p;
            } else
                *pix = 0xF54A004A;

            // Go to next pixel.
            pix++;
        }
        // Go to next line.
        pix += linePad;
    }

    return SlDrawReturn_configure;
}



/* Replace this function with something useful. */

void
draw_bitmap(const FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;


  /* for simplicity, we assume that `bitmap->pixel_mode' */
  /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;

      text_image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}


int main(void) {

    // We need to know this does not change, so that we know how to draw.
    ASSERT(SLATE_PIXEL_SIZE == 4);

    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    FT_Library library;
    FT_Face face;

    FT_GlyphSlot  slot;
    FT_Matrix matrix;
    FT_Vector pen; /* untransformed origin  */
    FT_Error error;

    // apt install fonts-freefont-otf
    //
    char *filename = "/usr/share/fonts/opentype/freefont/FreeSans.otf";
    char *text = "Hello World!";

    const double angle = ( 25.0 / 180 ) * M_PI;

    int target_height = HEIGHT;
    int n;
    const int num_chars = strlen(text);

    error = FT_Init_FreeType( &library );              /* initialize library */
    RET_ERROR(!error, 1, "FT_Init_FreeType() failed");

    error = FT_New_Face(library, filename, 0, &face);/* create face object */
    RET_ERROR(!error, 1, "FT_New_Face() failed");

    /* use 50pt at 100dpi */
    error = FT_Set_Char_Size(face, 50 * 64, 0, 100, 0 ); /* set character size */
    RET_ERROR(!error, 1, "FT_Set_Char_Size() failed");

    /* cmap selection omitted;                                        */
    /* for simplicity we assume that the font contains a Unicode cmap */

    slot = face->glyph;

    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 cartesian space coordinates; */
    /* start at (300,200) relative to the upper left corner  */
    pen.x = 60 * 64; // 60 was 300
    pen.y = ( target_height - 300 ) * 64; // 300 was 200


    for ( n = 0; n < num_chars; n++ ) {
        /* set transformation */
        FT_Set_Transform( face, &matrix, &pen);

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
        RET_ERROR(!error, 1, "FT_Load_Char() failed");

fprintf(stderr, " %d ", slot->bitmap_left);
        /* now, draw to our target surface (convert position) */
        draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

        /* increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }

fprintf(stderr, "\n");

    struct SlDisplay *d = slDisplay_create();
    if(!d) return 1; // fail

    struct SlWindow *w = slWindow_createToplevel(
            d, 800, 700, 10, 10, draw, 0,
            SL_SHOWING);
    if(!w) return 1; // fail

#ifdef LOOP
    while(slDisplay_dispatch(d));
#endif

    // Valgrind shows that the libfreetype2.so is cleaned up.  Removing the
    // FT_Done_FreeType() line below makes the valgrind test fail.
    //
    // To test with Valgrind we run:
    //
    //   ./valgrind_run_tests 102_helloFreeType

    FT_Done_Face    ( face );
    FT_Done_FreeType( library );

    return 0;
}
