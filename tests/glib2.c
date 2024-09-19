/* Running with valgrind shows that libglib-2.0.so leaks:

   $  ./valgrind_run_tests glib2

  Shows 9 memory allocations that are not freed.
*/

#include <stdio.h>
#include <glib.h>

int main(void) {

    // We just need to access a symbol that is in libglib-2.0.so; without
    // that (g_print below) the linker stage will not link this with
    // libglib-2.0.so.
    fprintf(stderr, "g_print=%p\n", g_print);

    return 0;
}
