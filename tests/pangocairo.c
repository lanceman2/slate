/* Simple example to use libpangocairo to render text */
//
// Most of this code was taken from pango source:
// pango/examples/cairosimple.c
// https://github.com/GNOME/pango/blob/main/examples/cairosimple.c

/* running with valgrind shows that libpangocairo-1.0.so leaks system
 * resources:

   $  ./valgrind_run_tests pangocairo

*/

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <pango/pangocairo.h>

#include "../lib/debug.h"

cairo_status_t
write_func(void *user_data,
        const unsigned char *data,
        unsigned int length) {

    do {
        ssize_t w = write(1, data, length);
        if(w <= 0) {
            ERROR("write failed");
            exit(1);
        }
        ASSERT(w <= length);
        length -= w;
        data += w;
    } while(length);

    return CAIRO_STATUS_SUCCESS;
}

static void
draw_text(cairo_t *cr) {

#define FONT "Sans"
#define FONT_SIZE 110

    cairo_translate(cr, 500, 0);

    /* Create a PangoLayout, set the font and text */
    PangoLayout *layout = pango_cairo_create_layout(cr);

    pango_layout_set_text(layout, "Test is good!", -1);

    PangoFontDescription *desc = pango_font_description_from_string(FONT);
    pango_font_description_set_absolute_size(desc, FONT_SIZE * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    cairo_set_source_rgba(cr, 1.0, 0, 1.0, 1.0);

    //pango_cairo_update_layout (cr, layout);

    pango_cairo_show_layout(cr, layout);

    /* free the layout object */
    g_object_unref(layout);
}

int main (void) {


//#define EXIT
#ifdef EXIT
    // Enabling this code and then making and running with
    // this with Valgrind show that libgobject-2.0, libglib-2.0, and
    // libpixman-1 leak lots of memory.  It looks like there are library
    // constructors that allocate memory that is not cleaned up in
    // library destructors.  Both GNOME and KDE (Qt) codes trend to not
    // cleanup memory from their libraries, and the developers know
    // this (in both GNOME and Qt).

    return 0;
#endif

    if(getenv("VaLGRIND_RuN"))
        // Skip running this test with valgrind by returning 123.
        //
        // TODO: This does not work, because valgrind fails due to the
        // memory leaks in libgobject-2.0 and so on.  It was required that
        // the running of the code below here was that cause of the
        // valgrind failure, but this fails as soon as this program
        // exits with the library destructors (in libgobject-2.0 .et all)
        // not cleaning up.  We need another interprocess communication
        // method (not just return status) to measure the failure.
        //
        return 123; // 123 == skip this test


    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1200, 600);

    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    draw_text(cr);
    cairo_destroy(cr);

    int fd[2];

    if(pipe(fd) != 0) {
        ERROR("pipe() failed");
        return 1;
    }

    pid_t pid = fork();

    if(pid == -1) {
        ERROR("fork() failed");
        return 1;
    }

    if(pid == 0) {

        // I'm the child
        close(fd[1]);
        cairo_surface_destroy(surface);

        if(dup2(fd[0], 0) != 0) {
            ERROR("dup2() failed");
            return 1;
        }
        close(fd[0]);
        DSPEW("running display");
        // By default the ImageMagick program "display" reads stdin.
        execlp("display", "display", (const char *) 0);
        ERROR("execlp((\"display\",,0) failed.  "
                "Is ImageMagick program \"display\" installed"
                " in your PATH?");
        return 1;
    }

    // I'm the parent

    if(dup2(fd[1],1) != 1) {
        ERROR("dup2() failed");
        return 1;
    }

    if(CAIRO_STATUS_SUCCESS !=
          cairo_surface_write_to_png_stream(surface, write_func, 0)) {
        ERROR("cairo_surface_write_to_png_stream() failed");
        return 1;
    }
    cairo_surface_destroy(surface);
    ASSERT(0 == close(1));
    ASSERT(0 == close(fd[1]));

    DSPEW("waiting for display pid=%u", pid);

    int stat = 0;

    if(waitpid(pid, &stat, 0) != pid || stat) {
        ERROR("wait status fail");
        return 1;
    }

    return 0;
}
