// This test run with valgrind shows that libgobject-2.0.so leaks
// like a stuck pig.  I've kind of had it with GNOME.  In any case you
// can't do modular coding with libgobject-2.0.so and hence you can't do
// modular coding GTK widgets API or pango text layout API; both depend on
// libgobject-2.0.so.  The same (kind of idea) is true for Qt ...
//
// It could be that libglib-2.0.so leaks too.  I think that would
// be very piss poor form that the GNOME utility library leaks.

#include <stdio.h>
#include <glib-object.h>

int main(void) {

    // We just need to access a symbol that is in libgobject-2.0.so.
    fprintf(stderr, "g_object_new=%p\n", g_object_new);

    return 0;
}
