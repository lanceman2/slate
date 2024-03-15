
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
#define SLATE_VERSION  (SLATE_STR(SLATE_MAJOR) "." SLATE_STR(SLATE_MINOR) "." SLATE_STR(SLATE_EDIT))


#ifndef SL_EXPORT
#  define SL_EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct SlApp;
struct SlWindow;

SL_EXPORT struct SlApp *slApp_create(void);
SL_EXPORT void slApp_destroy(struct SlApp *app);

SL_EXPORT struct SlWindow *slWindow_create(void);
SL_EXPORT void slWindow_destroy(struct SlWindow *window);

#ifdef __cplusplus
}
#endif
