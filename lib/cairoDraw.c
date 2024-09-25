#include <cairo.h>


// TODO: To force a libcairo.so symbol to be required in libslate.so:
//
void *cairoDummyPtr = cairo_image_surface_create_for_data;


