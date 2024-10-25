To edit this file see:
[Markdown-Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)

# The main idea

We want to do 2D drawing very quickly.  The wayland protocol may just help
us do that while letting us still have a desktop windowing environment
that lets us run preexisting programs at the same time as we do 2D drawing
very quickly.  By using a wayland client we should out perform X11 client
programs.  We can draw and color a pixels on the screen, and much more
directly than we ever could with X11 clients.  Each "drawing step" does
not necessitate a system call, like with an X11 client had a lot of socket
read/write (system) calls.

### GTK AND Qt RANT:
slate attempts to not leak system resources like GTK3 and Qt6.  GTK3 and
Qt6 make modular programming impossible (at least without dead code being
stuck loaded in your program).  Both the GTK3 and Qt6 libraries leak lots
of system resources.  They designed their "main loop" classes to never be
cleaned up so long as your program (process) is running.  Making class
objects without destructors; is bad form; especially when they are the
most important class objects in the whole API (in both Qt6 and GTK3).  I
have talked with their developers, and that's just how they ride.  I don't
expect newer versions to improve after talking with the developers.  If my
code sucks (in the robust code sense) please let me know.  I'm one of
those guys that demand perfection.  Code that leaks system resources (by
design and implementation) is not acceptable.

Looks like libgobject-2.0.so has lots of memory leaks which makes using
all the higher level (dependent) GTK libraries (linked in programs) leak
too.  libgobject-2.0.so allocates memory in the library constructor that
it does not cleanup in a library destructor; even when you never call an
API function. 

We have a test for testing libglib-2.0.so for leaks and it shows that
programs linked with libglib-2.0.so leak lots of system resources.

Run:

```sh
  $ cd tests && make && ldd ./glib2
```
After the make/build spew we see:
```
        linux-vdso.so.1 (0x00007ffd5f3de000)
        libglib-2.0.so.0 => /lib/x86_64-linux-gnu/libglib-2.0.so.0 (0x00007fcb9b019000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fcb9ae38000)
        libpcre2-8.so.0 => /lib/x86_64-linux-gnu/libpcre2-8.so.0 (0x00007fcb9ad9e000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fcb9acbf000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fcb9b17d000)
```
To see the leaks run (from tests/):
```sh
  $ ./valgrind_run_tests glib2
```


## This totally explains what wayland is

See this C code
[hello wayland](https://github.com/emersion/hello-wayland.git).

We are currently developing "slate" on Debian GNU/Linux 12 (bookworm) with
the Gnome 3 desktop (as of Sept 2024 on Debian 12 with KDE plasma).  An
older version of Gnome 3 desktop was missing symbols needed to compile
hello wayland client program without linking to a fuck ton of libraries
(at least 100 as I recall).  Now I can compile and run the "hello wayland"
program with just:
```sh
  $ ldd ./hello-wayland
```
```
	linux-vdso.so.1 (0x00007ffc2dcec000)
	libwayland-client.so.0 => /lib/x86_64-linux-gnu/libwayland-client.so.0 (0x00007f7a87371000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f7a87190000)
	libffi.so.8 => /lib/x86_64-linux-gnu/libffi.so.8 (0x00007f7a87184000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f7a8717f000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f7a873c1000)
```
as you can see it just links with 6 libraries.

Testing shows that it runs with just those libraries (it does not seem to
dynamically load more libraries); see by running:
```sh
  $ LD_DEBUG=files ./hello-wayland
```

## Wayland Example C Code With A Little More Than Hello

[wleird](https://github.com/emersion/wleird.git)
[wleird](https://gitlab.freedesktop.org/emersion/wleird)
Has stuff like cursor, resize, and subsurfaces.
Window resizing does not work with GNONE 3.

## Testing libwayland-client.so and libffi.so for system resource leaks


We'll add that as a test program in ./tests.  Looks like they do not leak.

We have many test programs that link with lib/libslate.so which we link
with both libwayland-client.so and libffi.so.

On wayland sub-surfaces:
https://ppaalanen.blogspot.com/2013/11/sub-surfaces-now.html


## Testing harfbuzz for system recourse leaks

Looking at
```sh
  $ tests/valgrind_run_tests harfbuzz
```
shows that programs linked with libharfbuzz depend on libglib-2.0 which
makes this test program leak.  I guess I will not be using libharfbuzz
with slate.

ref https://harfbuzz.github.io/index.html

## Cairo Drawing

Looks like Cairo can do drawing for libslate.so.  It would be nice to use
a libcairo.so that does not link with the family of X11 libraries, which
is a relatively large number of libraries that will not be needed.

It looks like the Cairo software package has meson build options to make
it so that the built libcairo.so library file will not depend on these X11
libraries.

We could git clone https://gitlab.freedesktop.org/cairo/cairo

We have a test program tests/cairoDraw that uses libslate.so with it
being linked with libcairo.so.

## List Of Code Change Requests

- [memory leak in wleird wayland client examples](
https://gitlab.freedesktop.org/emersion/wleird/-/issues/33)

- [memory leak in libpixman](
https://gitlab.freedesktop.org/pixman/pixman/-/issues/111)

I was somehow able to make these "Issues", but it appears that that is the
extent of what I'll be able to do.  I have a sense that these "Issues" may
not be seen as spam.

Looks like a gitlab service is as good as github, for what I need.
All the newer stuff that is in github.com is not so helpful for me.
That's good to see.

