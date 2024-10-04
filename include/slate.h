#ifndef __SLATE_H__
#define __SLATE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
\c SLATE_MAJOR is the major version number.
\c SLATE_MAJOR may be increased for a given release,
but not often. */
#define SLATE_MAJOR  0
/**
\c SLATE_MINOR is the minor version number.
\c SLATE_MINOR may be changed for a given release,
but not often. */
#define SLATE_MINOR  0
/**
\c SLATE_EDIT is the edit version number.
\c SLATE_EDIT should be changed for each release. */
#define SLATE_EDIT   5

// doxygen skips SLATE_STR and SLATE_XSTR
#define SLATE_STR(s) SLATE_XSTR(s)
#define SLATE_XSTR(s) #s

/**
\c SLATE_VERSION is the version of this slate software project
as we define it from the \c SLATE_MAJOR, \c SLATE_MINOR, and \c SLATE_EDIT.
*/
#define SLATE_VERSION  (SLATE_STR(SLATE_MAJOR) "." \
        SLATE_STR(SLATE_MINOR) "." SLATE_STR(SLATE_EDIT))

// This file may get installed in the "system" (or where ever installer
// decides) so we do not polute the CPP (C pre-processor) namespace by
// defining EXPORT, instead we define SL_EXPORT
#ifndef SL_EXPORT
#  define SL_EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct SlDisplay;
struct SlWindow;
struct SlWidget;
struct SlSurface;

#define SLATE_PIXEL_SIZE   (4)


// Child widget packing gravity (for lack of a better word)
//
// Is there a "field/branch" in mathematics that we can know that will
// give use a more optimal way to parametrise widget rectangle packing?
// We need to keep in mind that our rectangles can grown and shrink as
// needed to make the resulting window fully packed (with no gaps).
//
// Studying the behavior of gvim (the program) with changing multiple edit
// views is very helpful in studying widget containerization.  Like for
// example how it splits a edit wiew.  And, what happens when a view is
// removed from in between views.  And, what happen when a view (widget)
// border is moved when there are a lot of views in a large grid of
// views.
//
// As in general relativity, gravity defines how space is distributed
// among its massive pieces.  WTF.
//
enum SlGravity {

    // SlGravity is a attribute of a widget container (surface), be it a
    // widget or window.

    SlGravity_None = 0, // For non-container widgets or windows

    // The container surface can only have zero or one widget, so it puts
    // the child widget where ever it wants to, on its surface.
    SlGravity_One,

    // T Top, B Bottom, L Left, R Right
    //
    // Vertically column aligning child widgets
    SlGravity_TB, // child widgets float/align top to bottom
    SlGravity_BT, // child widgets float/align bottom to top

    // Horizontally row aligning child widgets
    SlGravity_LR, // child widgets float/align left to right
    SlGravity_RL,  // child widgets float/align right to left

    SlGravity_Callback // The window or widget will define its own packing.
};


// Widgets can be greedy for different kinds of space.
// The greedy widget will take the space it can get, but it has
// to share that space with other sibling greedy widgets.
//
// Q: If not no children in the tree are greedy in X (or Y too), then does
// that mean that the window will not be expandable in the X (Y)
// direction?  I'm thinking the answer is no; but there may be a need to
// have the idea of a windows natural (X and Y) size for this case.
//
enum SlGreed {
    // H Horizontal first bit, V Vertical second bit
    SlGreed_None = 00, // The widget is not greedy for any space
    SlGreed_H    = 01, // The widget is greedy for Horizontal space
    SlGreed_V    = 02, // The widget is greedy for Vertical space
    SlGreed_HV   = (01 & 02), // The widget is greedy for all 2D space
    SlGreed_VH   = (01 & 02)  // The widget is greedy for all 2D space
};


// The function symbols that the libslate.so library provides:
SL_EXPORT struct SlDisplay *slDisplay_create(void);
SL_EXPORT bool slDisplay_dispatch(struct SlDisplay *d);
SL_EXPORT void slDisplay_destroy(struct SlDisplay *d);

SL_EXPORT struct SlWindow *slWindow_createToplevel(struct SlDisplay *d,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        bool showing);
SL_EXPORT struct SlWindow *slWindow_createPopup(struct SlWindow *parent,
        uint32_t w, uint32_t h, int32_t x, int32_t y,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        bool showing);
SL_EXPORT void slWindow_setDraw(struct SlWindow *win,
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride));
SL_EXPORT void slWindow_destroy(struct SlWindow *w);

// Calculate all the widget (and window) geometries, positions (x,y)
// widths and heights; and that's all.  Just after calling this the widths
// and heights may not be consistent with what is currently being
// displayed.
//
// TODO: Remove this function from the user API.
//
SL_EXPORT
void slWindow_compose(struct SlWindow *win);

// Also compose if needed.  Note, the changes may not be visible on the
// computer monitor after this function call if it has dispatch==false.
//
// TODO: What about other dispatched events that come while we call
// wl_display_dispatch() in this function?  Queuing them in yet another
// queue (a slate event queue) would add complexity to libslate.so.
//
SL_EXPORT
void slWindow_show(struct SlWindow *win, bool dispatch);


// This not only defines the widget, it also packs it too.
//
// This is very limited, but that's the point: defining widgets with less
// code.  We try to create a slate widget with one slate API (application
// programming interface) function call.
//
// TODO: Add other event handlers, not just draw().
//
// Q: Are slate widgets given position, width, and height in units of root
// window pixels?  We could have widgets drawn across factions of a pixel
// via drawing with some kind of antialiasing.
//
// Cairo has surfaces that have discrete pixels.  It may not be obvious
// how to blend cairo surface edges between widgets.  Lets see, if we give
// widgets a full pixel to draw on for all the factions of a pixel; then
// the parent widget will blend all the extra vertical or horizontal
// edges, using a weighted (by pixel faction size) average of the color
// for the widget edges that are on non-integer multiples of a pixel.  The
// child widget could be told that it has a "factional" size, or should it
// just think it has that extra pixel piece?
//
// Also, we need to consider that cases when we do not use cairo to draw.
//
// It looks like GTK (and Qt) does not consider having widgets with
// non-discrete (fractional) pixel sizes.
//
// Looks like on the wayland compositor that I'm using the pointer motion
// event pass a fixed point number that appears to never convert to a
// floating point double that has a fractional part.  That could just be
// because of the nature of my desktop display screen.
//
// Okay, putting my foot down, I decree: slate widgets are rectangles of a
// discrete number of pixels.  I guess it's zero pixels when the widget is
// squished.
//
// Do we want a widget boundary/resize action built into the boundaries
// between widgets?
//
// Is there enough widget parameters specified here to make a reasonably
// interactive display of widgets (rectangles)?  Of course we can add more
// widget (and window) parameters with reasonable default values that are
// not passed in this function, that have various helper functions to set,
// get, or act (in some way) with these added widget (and window)
// parameters.
//
// Q: Should there be a automatic widget/window geometry saving/loading
// feature?  A dot file with an slate API user application namespace.
// Maybe on by default, with a default auto-generated application
// namespace.
//
// Widget builder function.
//
SL_EXPORT struct SlWidget *slWidget_create(
        // "parent" is either from a SlWindow or a SlWidget which both
        // have a SlSurface in them.  This parent owns this new widget.
        struct SlSurface *parent,
        // The width and height that the widget would like to use for
        // drawing itself and not for its children.  The size needed
        // for its children will be calculated from the children's
        // width, height, and this widgets border width (which pads
        // between the children).
        uint32_t width, uint32_t height,
        /* Children of this returned widget feel this gravity.
         * It's like the gravity in a room in 2D space.
         * Leaf widgets (non-container) have no gravity
         * (SlGravity_None). */
        enum SlGravity gravity,
        /* This returned widget is wanting this kind of 2D space. */
        enum SlGreed greed,
        // backgroundColor is only used if there is no draw function set.
        uint32_t backgroundColor, // A R G B with one byte for each one.
        // TODO: stuff like leftBorderWidth
        // TODO: CSS like interfaces
        // The idea of padding is internal to the widget (or window)
        // If there is padding the widget implementation needs to use it to
        // get the above width and height parameters.
        uint32_t borderWidth, // part of the container surface between
                              // children
        int (*draw)(struct SlWindow *win, uint32_t *pixels,
            uint32_t w, uint32_t h, uint32_t stride),
        bool hide);


// Should these next two functions be inline static?
SL_EXPORT
struct SlSurface *slWidget_getSurface(struct SlWidget *widget);
//
SL_EXPORT
struct SlSurface *slWindow_getSurface(struct SlWindow *window);


// TODO: remove this interface.
SL_EXPORT char *slFindFont(const char *exp);



// libfreetype.so wrapper functions.
//
// TODO: Make it for widgets too (not just windows).
//
SL_EXPORT bool slWindow_DrawText(struct SlWindow *win,
        const char *text, const char *font,
        int32_t x, int32_t y, uint32_t w, uint32_t h,
        double angle/*in radians*/,
        uint32_t bgColor,
        uint32_t fgColor);


#ifdef __cplusplus
}
#endif


#endif // #ifndef __SLATE_H__
