/* Running with valgrind shows that libharfbuzz.so does leak system
 * resources.  It links with libglib-2.0.so so, so of course it does.

   $  ./valgrind_run_tests harfbuzz

*/
// TODO: Are we sure that libharfbuzz.so is required to depend on
// libglib-2.0.so.  Could it be that there is a build option to
// harfbuzz not include a dependency on libglib-2.0.so?
// I think that is not likely, but it's a thought I have.

#include <stdio.h>
#include <hb.h>


int main(void) {

    fprintf(stderr, "hb_buffer_create=%p\n", hb_buffer_create);

    return 0;
}
