/* Running with valgrind shows that libpixman-1.so does leak system
 * resources.

   $  ./valgrind_run_tests pixman-1

*/

#include <stdio.h>
#include <pixman.h>


int main(void) {

    fprintf(stderr, "pixman_transform_init_identity=%p\n",
            pixman_transform_init_identity);

    return 0;
}
