/* Running with valgrind shows that libdecor-0.s does leak system
 * resources.

   $  ./valgrind_run_tests libdecor

*/

#include <stdio.h>
#include <libdecor-0/libdecor.h>


int main(void) {

    fprintf(stderr, "libdecor_decorate=%p\n",
            libdecor_decorate);

    return 0;
}
